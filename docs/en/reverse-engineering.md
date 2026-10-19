# Reverse engineering the SJRC client

[繁體中文](../zh/reverse-engineering.md) · English

Everything this project does on the wire was derived from the publicly distributed
`club.sjrc.app` 2.9.0 Android build and cross-checked against the public Swagger
document. This page records the method, the evidence, and the limits of the
conclusions — not just the final endpoint list.

> **Boundary.** The analysis is for interoperability and protocol understanding.
> The repository contains no live content keys, credentials, or user data. The
> full decompiled source tree is not redistributed; protocol evidence is presented
> through curated endpoint data and small wrapper excerpts. See [Legal and acceptable use](legal.md) and
> [NOTICE.md](../../NOTICE.md).

## Target

| Item | Value |
|---|---|
| Package | `club.sjrc.app` |
| Version | 2.9.0 (versionCode 120) |
| Distribution | `.apks` split set: base + arm64-v8a + xxhdpi + zh |
| `base.apk` | 116,959,933 bytes |
| DEX files | 6, about 53 MB total |
| Upstream reference smali classes (not distributed) | 4,888 |
| `minSdk` / `targetSdk` | 26 / 35 |
| Obfuscation | Partial — `club.sjrc.app.*` and `com.middle.*` keep their names |
| Repository references | `data/api-endpoints.csv/json`, this documentation, and `phone/src/` excerpts |

The class count is an upstream research statistic, not a claim that those files
are in this repository. The full decompiled tree and official APK are not
redistributed; this page documents conclusions alongside curated data and public
specifications.

## Research question and method

The question was narrow: **what exactly must a client send for the server to
credit reading time, and what does the official client do that is incidental?**

Three sources were used, in this order of trust:

1. **Smali from the shipped APK** — byte-level ground truth for constants,
   control flow, and field names.
2. **The public Swagger document** — authoritative for DTO shape, validation,
   and error codes, but not for the *sequence* the app uses.
3. **A live account** — used only to confirm units and success envelopes. No
   account data, token, or response body is checked in.

A claim was accepted only when at least two sources agreed, or when one source was
unambiguous (for example, a `const-wide/16 3` next to a 3000 ms countdown).

```text
SJRC_2.9.0.apks
  └─ unzip
       └─ base.apk
            ├─ aapt2 dump badging/xmltree → AndroidManifest.xml
            ├─ baksmali                   → external research reference (not distributed)
            ├─ androguard / jadx          → readable Java       (lossy; not shipped)
            ├─ strings / grep             → endpoint literals
            └─ swagger-json               → DTO and error cross-check
```

## Recovering the manifest and SDK levels

`aapt2 dump badging base.apk` gives the package, version, SDK levels, and launch
activity without executing anything. The official APK and decoded manifest are
not included; these values are recorded here as analysis notes.

Two different apps matter:

| App | Package | Version | minSdk | Purpose |
|---|---|---|---:|---|
| Official client | `club.sjrc.app` | 2.9.0 (120) | 26 | Protocol reference |
| This Android wrapper | `club.sjrc.autoread` | 1.0 (1) | 23 | WebView shell |

Do not mix their manifests. The wrapper's `minSdk 23` is a property of this
project, not evidence about SJRC's supported Android versions.

## Why smali, not Java

Decompiled Java is easier to read but loses generics and mis-expands Kotlin
`suspend` state machines and inline functions. For protocol work, small details
matter more than readability. The `time` field is the textbook example:

```smali
# BookReadViewModel.readTimeUsed()
iget v0, p0, ->errorTime:I
const-wide/16 v1, 3          ; base value
if-lez v0, +004h             ; skip the multiply when errorTime <= 0
int-to-long v3, v0
mul-long/2addr v1, v3        ; v1 = 3 * errorTime
```

The Java view might show a helper call or a `3L`; the smali shows that the value
is a **3-second base**, and that consecutive failures multiply it. That is why
the clients send `time: 3`, not `time: 3000` and not a timestamp.

## Client architecture

```text
UI          WelcomeLoginActivity · ReadActivity · MagazineActivity
ViewModel   BookReadViewModel              ← heartbeat core
Repository  HttpRepo (singleton)
Network     Retrofit + OkHttp
            ParameterInterceptor            header injection
            TokenExpireInterceptor          refresh on token errors, then retry
            ExceptionCatchInterceptor
Model       UseTimeEntity · ReadPosRequest · NetResponse
Crypto      content layer (described below, not implemented here)
```

### Headers: what is mandatory and what is incidental

`ParameterInterceptor` injects the same header family on every request. The
important ones are:

| Header | Value | Why it matters |
|---|---|---|
| `version` | `2.9.0` | Server-side compatibility gate |
| `x-current-platform` | `android` | Selects the client contract |
| `x-current-locale` | `zh-HK` (this project) | The official app derives it from `LanguageManage`; the server accepts the fixed value |
| `x-auth-schema` | `CLIENT` or `PUBLIC` | Chooses which auth rules apply |
| `authorization` | `Bearer <accessToken>` | **Only** for `CLIENT` and a non-empty token |

`x-auth-schema` is not optional. Sending a client route without it, or a token
to a `PUBLIC` route, produces `10104` rather than a generic 401. This was one of
the first findings that made the three clients possible.

### Two network styles coexist

```java
// 1. map builder — no type safety at all
new BodyBuilder().put("time", boxLong(3)).toBody()

// 2. typed DTO
new ReadPosRequest(bookId, new PosEntity(...))
```

`BodyBuilder` is a `HashMap` serialised with Gson. A wrong unit or field name is
only caught by the server at runtime, which is exactly how `10102` happens. The
typed DTOs are safer, but the official app does not use them consistently. When
the two disagree, the Swagger DTO wins for shape; the smali wins for what the app
actually sends.

## The heartbeat state machine

This is the core of the official client, recovered from `BookReadViewModel`:

```text
handReadTimeStart()
  POST /v1/client/reading-time/start { bookId }
    success → bookSessionId = data.sessionId, errorTime = 0

startCountDown()
  setMillisInFuture(9999999000)
  setCountDownInterval(3000)
  tick → handReadTimeUse()

handReadTimeUse()
  time = (errorTime > 0) ? 3 * errorTime : 3
  PATCH /v1/client/reading-time { sessionId, time }
    success → check quota
        !isUserQuotaValid()      → readStatus 2,  EventBus( 80008 )
        isRemainTimeLessThan20() → readStatus 13, EventBus(-777777 )
    failure → errorTime++
        errorTime > 3 → EventBus(-1001) forced logout

handReadTimeEnd()
  PATCH /v1/client/reading-time/end { bookId }        ← official client
```

Quota predicates:

```kotlin
isUserQuotaValid()       = totalQuota >= usedQuota
isRemainTimeLessThan20() = (totalQuota - usedQuota) <= totalQuota * 0.2f
```

The second name is misleading: it is a **ratio** check, not “20 minutes left”.
The client stops when 20% or less remains, even if that is several hours.

The `end` mismatch is a real defect in the official client: the Swagger
`EndReadingDto` requires `sessionId`, but the app sends `bookId`. The server
answers `80009` (“reading session not found”). This project deliberately sends
`sessionId` and documents the difference rather than copying the bug.

## Request pipeline

```text
call site
  → Retrofit
  → ParameterInterceptor
      if x-auth-schema == "CLIENT" and token non-empty
          authorization: Bearer <token>
      always
          x-current-locale: zh-HK
          x-current-platform: android
          version: 2.9.0
  → OkHttp   connect/read/write 60 s · followRedirects=false · no CertificatePinner
  → api.sjrc.club
  → NetResponse{ code, data, message, totalCount, isRefresh }
       isSucc() == (code == 200 || code == 201)
```

Two details are easy to miss:

- HTTP status does not decide success. A `200 OK` can carry `{code: 80009}` in the
  envelope, and the clients must read `code`.
- Token refresh is guarded in `TokenExpireInterceptor`. When refresh fails, the
  app forces a login rather than retrying forever; `20042` and `20043` are the
  codes that usually end the run.

## Recovering the endpoint list

Retrofit annotations live on interface methods. `data/` contains the curated list
of 98 methods and 67 unique `METHOD + path` pairs, retaining owning-class names
as research indexes. The full decompiled source is not included.

Three conventions matter when reading the table:

- A method with a runtime `@Url` has no literal path; the normalised row uses
  `(dynamic @Url)`.
- Path parameters remain `{id}` in the table; they are not expanded with sample
  values.
- An endpoint appearing in the table is not automatically used by this project.
  Only the routes listed in [API reference](api.md#endpoints-used-by-this-project)
  are called by the shipped clients.

For a separately obtained, legally acquired reference copy, reproduce the raw
endpoint search with (the repository itself does not contain `sjrc-source/`):

```bash
grep -rn "reading-time" sjrc-source/smali | head
grep -rn "reading-pos" sjrc-source/smali | head
grep -rn "accounts/users" sjrc-source/smali | head
```

Endpoint strings live in the DEX constant pool, so they may not appear in the
method body you are reading. Follow the field that holds the `@Url` annotation.

## What was verified, and how

| Claim | Evidence | Counter-check |
|---|---|---|
| `time` and quota are seconds | `readTimeUsed()` base 3; `setCountDownInterval(3000)`; live response values such as 10933 ≈ 3.04 h | Comparing `usedQuota` before/after a known number of heartbeats |
| `end` wants `sessionId` | Swagger `EndReadingDto`; official client sends `bookId` and gets `80009` | Route shape and error text |
| A code needs a ticket | `send-verifications` returns `{status:true}` with no ticket; `email/login` requires `{ticket, verificationCode}` | Trying a code without pre-login fails with “Missing ticket” |
| Google sign-in cannot run in a WebView | Google's OAuth policy returns `disallowed_useragent` | Android wrapper ships email/password only |
| Envelope success is 200/201 | `NetResponse.isSucc()` in smali | Live `{code:200}` responses |

Deliberately not verified: the content-encryption path end to end, any account
belonging to another person, server internals, or whether the platform permits
automation. Those are outside the project's scope and authority.

## Content encryption (described, key material redacted)

The official client contains a content-protection layer:

```text
file layer   AES-192-CBC, key hard-coded in the APK (per-file IV)
key layer    RSA-2048 OAEP-SHA256
             GET /v1/client/booksKey?bookId=N → { key: <RSA ciphertext>, url }
             decrypted with an RSA private key embedded in the APK
```

This is recorded only to explain why the official app can display protected books
and why this project does not. The repository does not include the full decompiled
classes or official APK, so it contains no material that can decrypt protected
content. The three clients never implement or
call this layer and never request `booksKey`. Adding a working content-access
path would move the project from protocol interoperability into circumvention
and is out of scope. See [Legal](legal.md#no-drm-circumvention).

### Key material and source scope

The full decompiled tree is not redistributed. `data/source-stats.json` separates
upstream research size from files actually included in this repository. The
original APK is not redistributed here either.

## Navigating the source

The following commands apply to a separately obtained reference copy; this
repository does not contain `sjrc-source/`:

```bash
# find a class
find sjrc-source/smali -path "*BookReadViewModel*"

# list methods
grep -n "^\.method" \
  sjrc-source/smali/club/sjrc/app/business/book/viewModel/BookReadViewModel.smali

# endpoint literals
grep -rn "reading-time" sjrc-source/smali | head
```

| Path | Contents |
|---|---|
| `club/sjrc/app/net/` | Retrofit service, interceptors, entities |
| `club/sjrc/app/business/book/viewModel/` | heartbeat state machine |
| `club/sjrc/app/business/login2/` | login flow |
| `club/sjrc/app/manager/` | `TokenManager`, `LanguageManage` |
| `com/middle/network/` | `BodyBuilder`, `NetResponse` |
| `com/skytree/epub/` | pagination engine — pages are computed here |

The small excerpts in `phone/src/*.smali` provide protocol context. The full
upstream reference tree is not included in this repository.

## Limits and maintenance

- This is a snapshot of 2.9.0. A future app release can change headers, units, or
  routes; re-derive before assuming the table still applies.
- Obfuscation is partial, so names are useful but not guaranteed stable.
- The live API is a third-party service. It can rate-limit, change validation, or
  disappear. The project has no backend fallback.
- No automated test contacts the real API. CI checks links, data consistency,
  syntax, and secrets only.

## Notice

Decompiled output of a publicly distributed APK, included for interoperability
research only. Copyright remains with the original owner. No keys, credentials,
tokens, or personal data are included. Open an issue to request removal.
