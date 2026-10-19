# API reference

[中文](../zh/api.md) · English

Base URL: `https://api.sjrc.club/`

Authoritative schema: <https://api.sjrc.club/swagger-json> (public, no token
required). The tables below are the subset this project relies on, verified
against the decompiled 2.9.0 client and live responses.

> The API is not ours. It can change without notice. Treat this page as a
> snapshot of 2.9.0, not a contract from SJRC.

## Transport

| Item | Value |
|---|---|
| Scheme | HTTPS only |
| Host | `api.sjrc.club` |
| Content type | `application/json` |
| Timeouts | web 30 s; Windows 8 s connect / 15 s receive; official client 60 s |
| Redirects | Windows: disabled. Web: browser default, but no redirects observed |
| Certificate pinning | none in this project or the captured client |
| CORS | The web build requires the API to allow the page origin; Google GIS needs its own origin allow-list |

The app never adds a query-string token. Credentials travel in headers or JSON
bodies only.

## Headers

| Header | Value | When |
|---|---|---|
| `x-auth-schema` | `CLIENT` or `PUBLIC` | Every request |
| `authorization` | `Bearer <accessToken>` | Only when schema is `CLIENT` and a token exists |
| `x-current-platform` | `android` | Every request |
| `x-current-locale` | `zh-HK` | Every request |
| `version` | `2.9.0` | Every request |
| `content-type` | `application/json` | Requests with a body |

Why `x-auth-schema` matters: the server has a guard that rejects a public route
when the schema says `CLIENT`, and a client route when the schema is missing or
`PUBLIC`. The captured interceptor also gates the Bearer header on
`schema == "CLIENT"`.

Observed responses:

```text
public route + x-auth-schema: CLIENT  -> 403 { code: 10104, message: "Forbidden resource" }
public route + no schema             -> 403 same
client route without Bearer          -> 401 { code: 10117, ... }
```

## Response envelope

```json
{
  "code": 200,
  "data": {},
  "message": "OK",
  "totalCount": 0,
  "isRefresh": false
}
```

The HTTP status is not the only signal. The clients treat a request as successful
when **the body `code` is 200 or 201**:

```js
isSucc(j) === (j.code === 200 || j.code === 201)
```

`data` is often an object, sometimes an array, and sometimes absent on `PATCH`.
`message` may be localised.

## Authentication

### Email + password

```http
POST /v1/public/accounts/users/email/pre-login
Content-Type: application/json
x-auth-schema: PUBLIC

{"email":"user@example.edu","password":"<base64 of the UTF-8 password>"}
```

Possible responses:

```json
{"code":200,"data":{"accessToken":"…","refreshToken":"…"}}
{"code":200,"data":{"ticket":"…"}}
```

If only `ticket` is returned, the account requires a verification code:

```http
POST /v1/public/accounts/users/email/login
{"ticket":"<ticket>","verificationCode":"123456"}
```

### Google

```http
POST /v1/public/accounts/users/google/pre-login
{"providerId":"<Google ID token>","type":"web"}
```

`type` must match the client class the credential was issued for. For a web GIS
credential, use `web`; an Android credential would use `android`, but this
project cannot register the Android package/SHA-1 with SJRC's Google project.
A mismatch usually returns `20002`.

The response shape is the same as email: `accessToken` or `ticket`, then:

```http
POST /v1/public/accounts/users/google/login
{"ticket":"<ticket>","verificationCode":"123456"}
```

### Refresh

```http
POST /v1/public/accounts/users/token/refresh
x-auth-schema: PUBLIC
{"refreshToken":"<refresh token>"}
```

Returns a new `accessToken` and usually a new `refreshToken`. A `20042` means the
refresh token is expired and the user must sign in again. A `20043` usually means
the token belongs to an older server session; clear it and sign in.

### Sending a verification code

```http
POST /v1/public/accounts/users/send-verifications
{"type":"email","identify":"user@example.edu"}
→ {"code":200,"data":{"status":true}}
```

This response contains **no ticket**. A code alone cannot be redeemed. The ticket
comes from `pre-login`, which proves the password/id_token step succeeded. This
is the cause of “missing ticket” errors when users press “send code” first.

## Endpoints used by this project

| Method | Path | Body | Notes |
|---|---|---|---|
| GET | `/v1/client/reading-time` | — | `totalQuota`, `usedQuota` in seconds |
| POST | `/v1/client/reading-time/start` | `{"bookId": 123}` | Returns `sessionId` |
| PATCH | `/v1/client/reading-time` | `{"sessionId":"…","time":3}` | `time` is seconds |
| PATCH | `/v1/client/reading-time/end` | `{"sessionId":"…"}` | Official client sends `bookId`; server returns 80009 |
| POST | `/v1/client/reading-pos` | `{"bookId":123,"pos":{...}}` | See shape below |
| GET | `/v2/client/books/{id}` | — | Title, authors, chapters |
| GET | `/v1/client/bookshelf/books?page=1&limit=50` | — | Preferred book source |
| GET | `/v1/client/homepage` | — | Fallback when the shelf is small |
| GET | `/v1/client/users/detail` | — | Profile; also validates a token |
| POST | `/v1/public/accounts/users/email/pre-login` | `{email,password}` | Base64 password |
| POST | `/v1/public/accounts/users/email/login` | `{ticket,verificationCode}` | |
| POST | `/v1/public/accounts/users/google/pre-login` | `{providerId,type}` | |
| POST | `/v1/public/accounts/users/google/login` | `{ticket,verificationCode}` | |
| POST | `/v1/public/accounts/users/send-verifications` | `{type,identify}` | No ticket |
| POST | `/v1/public/accounts/users/token/refresh` | `{refreshToken}` | |

The full extracted list is in [`data/api-endpoints.csv`](../../data/api-endpoints.csv):
98 Retrofit methods, 67 unique method+path pairs.

## Reading position payload

```json
{
  "bookId": 12345,
  "pos": {
    "chapterId": "3",
    "chapterLocation": 7,
    "position": 0.35
  }
}
```

| Field | Type | Meaning |
|---|---|---|
| `chapterId` | string | The official client sends the chapter index as a string |
| `chapterLocation` | integer | 1-based page inside the chapter |
| `position` | number | Page-in-chapter divided by pages-per-chapter, 4 decimals |

This project estimates pages. It does not parse the real layout. The server
accepts the shape; the value is approximate.

## Units

`totalQuota`, `usedQuota`, and `time` are **seconds**.

| Human value | Raw |
|---|---:|
| 8.00 hours allocated | `totalQuota: 28800` |
| 3.04 hours read | `usedQuota: 10933` |
| 4.96 hours left | `17867` |
| One heartbeat | `time: 3` |

`10933 / 3600 = 3.036`. If a UI says “minutes”, it is wrong by 60×.

## Error codes

| Code | Meaning | What the client does |
|---|---|---|
| `10102` | DTO validation failed | Log unit/session hint, count as error |
| `10104` | `x-auth-schema` mismatch / forbidden | Fix schema; do not retry blindly |
| `10117` | Token invalid | Refresh once, retry next tick |
| `10119` | Token expired | Refresh once, retry next tick |
| `20002` | Google ID token rejected | Check client type / origin |
| `20042` | Refresh token expired | Stop; sign in again |
| `20043` | Session not found | Clear stale refresh token; sign in |
| `20045` | Account not registered | Show message; no retry |
| `80008` | Quota exhausted | Stop |
| `80009` | Reading session not found | End must send `sessionId` |
| `-777777` | Remaining ≤ 20% | Warn; maybe continue |
| `-1001` | Forced logout | Stop after 4 consecutive failures |

Codes are integers and may be negative. Treat unknown codes as errors; log them
with the path and HTTP status.

## curl examples

Set variables in your shell; never paste a real token into a script that is
committed.

```bash
BASE=https://api.sjrc.club
TOKEN='<access token>'
COMMON=(-H "x-auth-schema: CLIENT" -H "x-current-platform: android" \
        -H "x-current-locale: zh-HK" -H "version: 2.9.0")

# quota
curl -sS "$BASE/v1/client/reading-time" "${COMMON[@]}" \
     -H "authorization: Bearer $TOKEN" | jq .

# start a session
curl -sS -X POST "$BASE/v1/client/reading-time/start" "${COMMON[@]}" \
     -H "authorization: Bearer $TOKEN" -H 'content-type: application/json' \
     -d '{"bookId":12345}' | jq .

# heartbeat (time is seconds)
curl -sS -X PATCH "$BASE/v1/client/reading-time" "${COMMON[@]}" \
     -H "authorization: Bearer $TOKEN" -H 'content-type: application/json' \
     -d '{"sessionId":"<session-id>","time":3}' | jq .

# end (sessionId, not bookId)
curl -sS -X PATCH "$BASE/v1/client/reading-time/end" "${COMMON[@]}" \
     -H "authorization: Bearer $TOKEN" -H 'content-type: application/json' \
     -d '{"sessionId":"<session-id>"}' | jq .
```

Public route example:

```bash
curl -sS -X POST "$BASE/v1/public/accounts/users/email/pre-login" \
  -H 'x-auth-schema: PUBLIC' -H 'content-type: application/json' \
  -d '{"email":"user@example.edu","password":"<base64>"}' | jq .
```

## Retry and refresh policy

- Web: refresh when a JWT is within 90 s of expiry, and on a 10117/10119 response.
- Windows: refresh every 150 s during a run, and on a 10117/10119 response.
- A failed refresh is fatal for the run; ask the user to sign in again.
- Do not retry 10102/10104/80008 in a tight loop. Fix the request or stop.
- Back off if the service returns rate-limit responses; the project has no
  built-in exponential backoff beyond the fixed tick interval.

## What is intentionally absent

- No `send-verifications` auto-call; the user must trigger it in the official
  flow.
- No attempt to obtain `booksKey`, decrypt a book, or call download endpoints.
- No attempt to bypass quota, CAPTCHA, or account state.
- No certificate pinning, so a local TLS-intercepting proxy can read traffic. Do
  not run the app on a network you do not trust.
