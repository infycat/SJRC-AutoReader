# Code map

[繁體中文](../zh/code-map.md) · English

Where each behaviour lives, what must change together, and which invariants are
easy to break. Line numbers refer to the checked-in files at version 1.0.0; when
they drift, the function names and tables are the stable part.

> The single-file narrative version of this map is
> [`CODE-OVERVIEW.md`](../../CODE-OVERVIEW.md). This page is the task-oriented
> index: “I want to change X — which files do I open?”

## Repository at a glance

| Path | Role | Entry points |
|---|---|---|
| `web/SJRC_AutoReader.html` | Complete web product. HTML, CSS, JavaScript, icon in one file. | `api()` at line 361, `startRun()` at 1065 |
| `phone/SJRC_AutoRead.apk` | Signed WebView shell, package `club.sjrc.autoread`. | `club.sjrc.auto.MainActivity` |
| `phone/src/index.html` | Page embedded in the APK; adds email/password sign-in and `__appPause`, and removes the browser-only Google flows. | `emailPreLogin()` at line 1302, `emailLoginByCode()` at 1336 |
| `phone/src/*.smali` | Six official-client excerpts used as protocol evidence. | `BookReadViewModel`, `ParameterInterceptor`, … |
| `pc/src/main.cpp` | Entire native Windows client. | `wWinMain`, `startRun()` at 1404, `workerLoop()` at 806 |
| Upstream `smali/` path (not included) | 4,888-class 2.9.0 reference used during analysis; count is research metadata only. | `club/sjrc/app/net/`, `.../book/viewModel/` |
| `data/` | Normalised endpoint tables and build statistics. | `api-endpoints.{csv,json}`, `source-stats.json` |
| `docs/en`, `docs/zh` | Bilingual user and developer documentation. | `docs/README.md` |
| `scripts/` | Offline checker, deterministic packager, local web server. | `check_repo.py`, `package_release.py` |

## The shared contract

No code is shared between the three clients, so these values are duplicated and
must be changed together. The checker verifies that the documentation and data
files agree with the binary hashes, but it cannot see semantic drift inside code.

| Contract item | Value | Web | Windows | Android |
|---|---|---|---|---|
| API base | `https://api.sjrc.club/` | `BASE`, line 293 | `API_HOST` | embedded page |
| Version header | `2.9.0` | `VER` | `API_VER` | embedded page |
| Platform header | `android` | `api()` | `httpCall()` | embedded page |
| Locale header | `zh-HK` | `api()` | `httpCall()` | embedded page |
| Auth schema | `CLIENT` / `PUBLIC` | per call | per call | embedded page |
| Heartbeat | 3 s, `time: 3` s | `heartbeat()`, 889 | `workerLoop()`, 806 | embedded page |
| Page turn | 20 s default | `turnPage()`, 1024 | `workerLoop()`, 806 | embedded page |
| Session close | `end {sessionId}` | `endBook()`, 872 | `endSession()`, 613 | embedded page |
| Token storage | local | `localStorage` | `HKCU\Software\SJRC\AutoReader` | WebView storage |
| User agent | browser/default | browser | WinHTTP default | WebView default |

The Android asset is a near-copy of the web page. If you change protocol logic in
`web/SJRC_AutoReader.html`, diff it against `phone/src/index.html` and decide
whether the mobile variant needs the same change. There is no automatic sync.

## Web build

`web/SJRC_AutoReader.html` is organised as one `<style>` block, one markup tree,
and one `<script>` block. Everything below is inside that script.

### State and formatting

| Symbol | Line | Responsibility |
|---|--:|---|
| `BASE`, `VER` | 293 | API origin and version header |
| `S` | 296 | All mutable runtime state: tokens, book, page, `sessionId`, counters, demo flag, quota snapshot |
| `fmtHours()` / `fmtHMS()` | 332 / 336 | Format **seconds** as hours or `h m s` |
| `log()` | 343 | Append a timestamped line, cap the panel at 400 lines |
| `netPill()` | 354 | Network indicator in the log card |
| `isSucc()` / `codeOf()` / `msgOf()` | 398–400 | Envelope helpers; success is 200 or 201 |

`S` mixes persisted values (token, refresh token, session recovery) with derived
counters. When adding a field, decide whether it must survive a reload; if yes,
add an explicit storage read/write and a cleanup path. Do not rely on object
serialisation of `S` as a whole — it also contains timers and DOM references.

### HTTP and auth

| Symbol | Line | Responsibility |
|---|--:|---|
| `api()` | 361 | Adds headers, Bearer token for `CLIENT`, timeout via `AbortController`, parses JSON, updates net pill |
| `refreshAccessToken()` | 403 | `POST token/refresh`; updates both tokens; does not force a retry loop |
| `initGsi()` / `tryPromptOneTap()` | 457 / 508 | Loads Google Identity Services and renders the button when a Client ID exists |
| `googleSignInClick()` / `onGoogleCredential()` | 524 / 551 | Opens the account chooser and handles the GIS credential callback |
| `preLogin()` | 591 | `google/pre-login`; either adopts tokens directly or stores a ticket |
| `adoptLogin()` | 621 | Persists tokens, clears stale tickets, moves to the account step |
| `doLogin()` | 641 | `google/login` with ticket + verification code |
| `loadAccount()` / `loadReadingTime()` | 658 / 683 | Reads account and quota snapshot |

Header rules to preserve: `version`, `x-current-platform`, `x-current-locale` are
always present; `x-auth-schema` is explicit; `authorization` is attached **only**
when the schema is `CLIENT` and the token is non-empty. Public login and refresh
calls must not leak a stale bearer token.

### Books and session lifecycle

| Symbol | Line | Responsibility |
|---|--:|---|
| `fetchCandidateBooks()` | 732 | Bookshelf first, homepage fallback; flattens nested book nodes |
| `fetchBookDetail()` | 758 | `/v2/client/books/{id}`; title, cover, chapters |
| `startBook()` | 781 | Creates the server session, flattens chapters, sends initial position |
| `sendReadPos()` | 840 | Builds `{bookId, pos:{chapterId, chapterLocation, position}}` |
| `endBook()` | 872 | `PATCH end {sessionId}` and clears `sjrc.session` |
| `heartbeat()` | 889 | `PATCH reading-time {sessionId, time}` with `time` in seconds |
| `onHeartbeatError()` | 942 | Token refresh for 10117/10119; counts other failures; stops after four |
| `updateProgress()` | 968 | Reconciles locally reported seconds with server delta |

`startBook()` is the only place that may set `S.sessionId`. `endBook()` is the only
place that may clear it after a successful start. If a future change adds another
path, make it go through these two functions or session accounting will diverge.

### Run loop and UI

| Symbol | Line | Responsibility |
|---|--:|---|
| `tick()` | 991 | One-second driver; re-entrancy guard; schedules heartbeat and turn |
| `turnPage()` | 1024 | Advances page; at end of book, chooses a random next book or ends the run |
| `startRun()` | 1065 | Validates target and token, resets counters, starts timer |
| `stopAll()` | 1112 | Idempotent stop: clears timer, re-enables buttons, calls `endBook()` |
| `renderPage()` | 860 | Page/chapter counters and progress bar |
| storage recovery | 1179 | On load, ends a stale session left in `localStorage` |

`tick()` uses a `tickBusy` guard. Do not remove it: heartbeat awaits network I/O,
and a naive `setInterval` would overlap requests when the API is slow. The loop’s
logical order is: target check → heartbeat when due → page turn when due →
progress update.

### Storage keys

| Key | Store | Contents | Cleared by |
|---|---|---|---|
| `sjrc.token` | `localStorage` | Access token | Logout, manual clear, browser data clear |
| `sjrc.refresh` | `localStorage` | Refresh token, if the server returned one | Logout, failed refresh, manual clear |
| `sjrc.ticket` | `sessionStorage` | One-shot login ticket | Successful exchange, tab close |
| `sjrc.session` | `localStorage` | `{sessionId, bookId}` for crash recovery | Successful `end`, logout, recovery path |

## Android wrapper

The checked-in APK contains six classes: `MainActivity`, `OAuthClient`,
`PopupChrome`, `Poster`, `Probe`, and `WebServer`. Only the manifest and an
excerpt of `MainActivity` are in the source tree; disassemble `classes.dex` for
the full shell.

| Area | Fact |
|---|---|
| Package | `club.sjrc.autoread` (distinct from official `club.sjrc.app`) |
| SDK | `minSdk 23`, `targetSdk 33` |
| Permissions | `INTERNET`, `ACCESS_NETWORK_STATE` |
| Launcher | `club.sjrc.auto.MainActivity` (fully qualified in the manifest) |
| Content | `file:///android_asset/index.html` |
| WebView flags | file access + universal access from file URLs enabled so the page can call HTTPS |
| Lifecycle | `onStop` calls JavaScript `__appPause()`; that path ends the session |
| Signing | V1 + V2 + V3; the APK is already signed |
| Sign-in | Email/password only; Google OAuth is blocked inside WebView |

`source` for the wrapper is partial by design. Rebuilding it means recreating the
six classes or replacing the shell; see [Building](building.md#android-wrapper).
The embedded page is the product logic; the shell only hosts it and forwards the
pause event.

## Windows build

`pc/src/main.cpp` is a single translation unit with a small JSON parser, WinHTTP
client, UI, registry storage, login helper, and worker thread.

| Subsystem | Lines | Notes |
|---|--:|---|
| Constants, colours, control IDs | 1–80 | `API_HOST`, `API_VER`, UI palette, `WM_APP_*` messages |
| JSON value and parser | 116–262 | Minimal recursive-descent parser; depth limit 64; no external dependency |
| `httpCall()` | 276 | WinHTTP: 60 s timeouts, no redirects, TLS, headers, body |
| `api()` wrapper | around 414 | JSON parse, log, success classification |
| Registry | 385–408 | `HKCU\Software\SJRC\AutoReader`; `target`, `hb`, `turn`, `token`, `refresh` |
| Refresh | 439 | `POST token/refresh`, updates registry after success |
| `sendReadPos()` | 594 | Reading position payload |
| `endSession()` | 613 | `PATCH end {sessionId}` |
| Worker state | 340–380 | `Snap` shared through `g_mtx`; counters and quota |
| `workerLoop()` | 806 | Start book, heartbeat, page turn, quota/error stop conditions |
| Local login server | around 1466 | Ports 47821/8080; idle shutdown; ticket/code exchange |
| `startRun()` / `stopRun()` | 1404 / 1440 | Validate UI, snapshot settings, join worker |
| Window procedure | around 2300 | Commands, painting, timer, close handling |

Threading rule: UI state lives in `Snap` and is copied under `g_mtx`; do not touch
HWNDs from the worker. The worker only posts `WM_APP_STATE` and appends log lines
through the marshalling helpers. `WM_CLOSE` asks the worker to stop and waits for
`endSession()` before destroying the window.

## Reference source (smali)

The paths below describe locations in the upstream research reference, not files
present in this repository. The curated endpoint data and analysis notes are in `data/`.

| Path | What it contains |
|---|---|
| `sjrc-source/smali/club/sjrc/app/net/MainHttpService.smali` | Retrofit endpoint declarations, including `reading-time`, `reading-pos`, `books`, `bookshelf` |
| `.../net/interceptor/ParameterInterceptor.smali` | Header injection: version, locale, platform, `x-auth-schema`, Bearer token |
| `.../net/interceptor/TokenExpireInterceptor.smali` | Refresh-and-retry behaviour on token errors |
| `.../business/book/viewModel/BookReadViewModel.smali` | Reading session, 3-second countdown, `time` calculation, quota predicates, forced logout |
| `.../manager/TokenManager.smali` | Token persistence and clearing |
| `../com/middle/network/BodyBuilder.smali` | Map-based request bodies (no type safety) |
| `../com/middle/network/NetResponse.smali` | Envelope and `isSucc()` semantics |
| `../com/skytree/epub/` | Pagination engine; pages are computed here, not by the server |

Use `grep -rn` for endpoint strings: they live in the DEX constant pool and may
appear in the service interface rather than a method body. The endpoint table in
`data/` is a normalised extraction, not a hand-written guess; if you change it,
record the source method in the `class` column.

## Data and generated artifacts

| File | Shape | Source of truth |
|---|---|---|
| `data/api-endpoints.csv` | 98 rows: `http,path,method,class,description` | Upstream 2.9.0 annotations (source tree not included) |
| `data/api-endpoints.json` | 98 methods, 67 unique method+path pairs | same |
| `data/api-endpoints-by-module.md` | Grouped summary | generated from the JSON |
| `data/source-stats.json` | Version, SDKs, upstream counts, included-source count, artifact hashes | Research notes and release binaries |
| `SHA256SUMS` | Download hashes | artifacts as shipped |

The JSON includes a synthetic `(dynamic @Url)` path for methods whose URL is
provided at runtime. Keep that convention if you regenerate the table; otherwise
the row count and the “unique paths” number will silently change.

## Where to change common things

| Goal | Open | Also check |
|---|---|---|
| Change heartbeat interval | `web/...:889` and `pc/src/main.cpp:806` | `phone/src/index.html`, configuration docs, official 3 s evidence |
| Add an endpoint | `data/api-endpoints.*`, web `api()` call sites, PC `api()` wrapper | API docs, `MainHttpService.smali`, error handling |
| Change default target | Web form default, PC `startRun()` fallback | start-run docs, screenshots, configuration table |
| Change token storage | Web: `localStorage` keys; PC: registry helpers | privacy/security docs, logout and recovery paths |
| Add a stop condition | Web `tick()`/`onHeartbeatError()`; PC `workerLoop()` | usage docs, error table, tests |
| Adjust page size | `pagesPerChapter` in the form | `startBook()`, position math, README warning |
| Add a language | Create paired `docs/en/X.md` and `docs/zh/X.md` | `docs/README.md`, checker parity rule, README index |
| Change the wrapper | APK manifest and `MainActivity` | mobile asset diff, signing, lifecycle pause |

## Invariants and traps

1. **Seconds, not milliseconds.** `time: 3` and `totalQuota: 28800`. Sending
   milliseconds produces `10102` or absurd progress.
2. **`sessionId`, not `bookId`, for heartbeat and end.** The official client's
   `end` call is wrong; do not copy it.
3. **One run loop per client.** The web `tickBusy` flag and the Windows
   `g_worker.joinable()` check enforce this. Two loops can double-count.
4. **Bearer only for `CLIENT`.** Sending a token to a `PUBLIC` route or omitting
   `x-auth-schema` produces `10104`.
5. **A ticket is one-shot.** Do not retry the same ticket after a failed code
   exchange; start a new pre-login.
6. **Google cannot run in the Android WebView.** Do not “fix” the missing button
   by loading GIS inside the wrapper.
7. **No live keys, no content.** The DRM layer is described for research only;
   do not add working decryption code or live key material to this repository.
