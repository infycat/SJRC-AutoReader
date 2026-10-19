# Glossary

[繁體中文](../zh/glossary.md) · English

Terms are grouped by where you meet them. The “Used in” column points at the
first document to read; it is not an exhaustive index.

## Core protocol

| Term | Meaning | Used in |
|---|---|---|
| **Heartbeat** | The periodic `PATCH /v1/client/reading-time` request that tells the server “this session is still reading”. The official client sends one every 3 seconds with `time: 3`. | [API](api.md) |
| **Reading session** | The server-side object created by `reading-time/start`, identified by `sessionId`. Heartbeats, positions, quota, and `end` all refer to it. | [Usage](usage.md) |
| **`sessionId`** | Opaque identifier returned by `start`. Required by the heartbeat and by `end`; **not** the same as `bookId`. | [API](api.md#endpoints-used-by-this-project) |
| **`bookId`** | Identifier of a book. Used by `start`, `books/{id}`, and `reading-pos`; the official client mistakenly sends it to `end`, producing `80009`. | [Troubleshooting](troubleshooting.md#80009-on-end) |
| **Reading position** | The `{chapterId, chapterLocation, position}` object sent by `POST /v1/client/reading-pos` during page turns. | [API](api.md#reading-position-payload) |
| **Quota** | The time budget the server tracks as `totalQuota` / `usedQuota`, in **seconds**. `28800` is 8 hours. | [API](api.md#units) |
| **Target hours** | The user-facing goal in hours. The clients convert it to seconds before comparing with quota. | [Configuration](configuration.md) |
| **`x-auth-schema`** | Request header selecting the auth contract: `CLIENT` for authenticated client routes, `PUBLIC` for login/refresh routes. | [API](api.md#headers) |
| **Bearer token** | `accessToken`, sent as `authorization: Bearer <token>` only when the schema is `CLIENT` and the token is non-empty. | [Security](security.md) |
| **Refresh token** | Longer-lived token exchanged at `token/refresh` for a new access token. Expiry is `20042`. | [API](api.md#refresh) |
| **Ticket** | Short-lived, one-shot value returned by `pre-login`. It must be combined with the verification code at `email/login` or `google/login`; a code alone is not enough. | [API](api.md#authentication) |
| **`code` / `isSucc`** | The JSON envelope's business status. Success is `code == 200 || code == 201`; HTTP status alone is not sufficient. | [API](api.md#response-envelope) |
| **`errorTime`** | Consecutive heartbeat failures. The official client multiplies the base `time` by this value and forces a logout after four failures (`-1001`). | [Reverse engineering](reverse-engineering.md#the-heartbeat-state-machine) |
| **`-777777`** | Internal event meaning “remaining quota is at or below 20% of total”. The name is a ratio check, not 20 minutes. | [Troubleshooting](troubleshooting.md#-777777--1001) |
| **`80008`** | Server code for quota exhausted. Stop the run; retrying will not create quota. | [Troubleshooting](troubleshooting.md#quota-and-progress) |
| **`80009`** | Server code for “reading session not found”. Most often caused by sending `bookId` to `end` instead of `sessionId`. | [Troubleshooting](troubleshooting.md#80009-on-end) |

## Clients and builds

| Term | Meaning | Used in |
|---|---|---|
| **Web build** | Single-file `web/SJRC_AutoReader.html`. Product and source are the same file; no build step. | [Architecture](architecture.md#web-build) |
| **Android wrapper** | `phone/SJRC_AutoRead.apk`, package `club.sjrc.autoread`, a WebView shell around an embedded copy of the page. Not the official app. | [Installation](installation.md#android) |
| **Windows build** | `pc/SJRC_AutoReader.exe`, native Win32/WinHTTP, statically linked, registry-backed settings. | [Architecture](architecture.md#windows-build) |
| **Official client** | The SJRC Android application `club.sjrc.app` 2.9.0 (versionCode 120), used as protocol reference. | [Reverse engineering](reverse-engineering.md#target) |
| **WebView** | Android's embedded browser component. Used by the wrapper; Google OAuth refuses to run inside it. | [Troubleshooting](troubleshooting.md#google-sign-in-missing) |
| **GIS** | Google Identity Services, the JavaScript library that returns an ID token for the web build. Requires a registered JavaScript origin. | [Configuration](configuration.md#web-and-android-fields) |
| **Demo mode** | Offline UI exercise. Sends no network requests and does not affect quota. | [Usage](usage.md) |
| **Local sign-in helper** | Windows-only temporary localhost page used to obtain a token; ports 47821 and 8080. | [Usage](usage.md#windows-local-sign-in-helper) |

## Repository and source

| Term | Meaning | Used in |
|---|---|---|
| **Smali** | Human-readable disassembly of DEX bytecode. The 4,888-class count describes the upstream research reference; the full tree is not included. | [Reverse engineering](reverse-engineering.md#why-smali-not-java) |
| **APK / APKS** | Android package / split-package set. The official 2.9.0 distribution is an APKS with base + ABI + density + language splits. | [Reverse engineering](reverse-engineering.md#target) |
| **Retrofit** | HTTP client library used by the official app; endpoints are declared as annotated Java methods. | [API](api.md#transport) |
| **OkHttp** | HTTP transport under Retrofit. The official client configures 60-second timeouts, no redirects, and no certificate pinning. | [Reverse engineering](reverse-engineering.md#request-pipeline) |
| **Interceptor** | OkHttp hook that adds headers or refreshes tokens. `ParameterInterceptor` and `TokenExpireInterceptor` are the two that matter here. | [Reverse engineering](reverse-engineering.md#client-architecture) |
| **BodyBuilder** | The official app's map-to-JSON helper. It has no type safety, which is why a wrong unit or field only fails at runtime. | [Reverse engineering](reverse-engineering.md#two-network-styles-coexist) |
| **DTO** | Data transfer object; the typed request/response shape. `EndReadingDto` expects `sessionId`. | [API](api.md#endpoints-used-by-this-project) |
| **DRM** | Digital rights management. The official client has a content-encryption layer. The three clients do not implement it; the public mirror ships no working keys. | [Legal](legal.md#no-drm-circumvention) |
| **Manifest** | `AndroidManifest.xml`, which declares package, SDK levels, permissions, and activities. Decoded copies are kept for both apps. | [Building](building.md#android-wrapper) |
| **Golden file** | A checked-in expected output used to detect accidental changes. The endpoint tables and checksums act as lightweight golden files. | [Testing](testing.md) |

## Conventions

- “Official” always means SJRC 2.9.0, never this repository.
- Times in JSON are **seconds** unless the text says milliseconds.
- `club.sjrc.app` is the original package; `club.sjrc.autoread` is this wrapper.
- `→` marks a request, `←` marks a response or a log line derived from one.
- File paths are relative to the repository root unless stated otherwise.
