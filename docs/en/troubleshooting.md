# Troubleshooting

[中文](../zh/troubleshooting.md) · English

Start with the exact message. The log shows a request path and a body `code`; the
code is more useful than the HTTP status.

## Quick diagnosis

| Symptom | Likely cause | Fix |
|---|---|---|
| Web page is blank | Opened with `file://`, or script error | Serve over HTTP; open DevTools |
| Google button does nothing | Origin not registered, Client ID empty, popup blocked | Add `http://localhost:8080`; paste Client ID |
| `disallowed_useragent` | Google OAuth inside a WebView | Use email/password (Android has no Google) |
| `Missing ticket` | Code entered without a `pre-login` ticket | Do the password step first; request a new code |
| `10102` | `time` in ms, missing `sessionId`, wrong JSON | Send seconds; check body |
| `10104` | `x-auth-schema` mismatch | Public route: `PUBLIC`; client route: `CLIENT` |
| `10117` / `10119` | Token invalid/expired | Refresh; if refresh fails, sign in again |
| `20002` | Google ID token rejected | Use a web client ID with origin `localhost:8080` |
| `20042` / `20043` | Refresh expired / stale session | Clear storage, sign in again |
| `80008` | Quota exhausted | Stop; wait for a new quota period |
| `80009` | `end` sent `bookId` | Send `sessionId` |
| Quota never moves | Demo mode, start failed, wrong account | Check log for start; check account |
| Target stops immediately | Target already reached or 0 | Set a positive target; restart |
| Android “App not installed” | Signature conflict with official app | Different package; uninstall conflicting build |
| Windows SmartScreen | Unsigned binary | Verify SHA-256, then “Run anyway” |
| Windows 8080 occupied | Another server / helper | Close it, or use manual token import |
| App works, then stops | 4 heartbeat failures, quota, token | Read the last 10 log lines |

## Web

### Blank page

1. Check the URL: `file:///...` will load the HTML but Google GIS and some fetch
   behaviour fail. Use:

   ```bash
   cd web
   python3 -m http.server 8080
   ```

2. Open DevTools → Console. A syntax error means the file was edited incorrectly.
3. Check the Network tab for a failed request. `net::ERR_CONNECTION_REFUSED`
   means the server is not running.
4. Hard-reload (`Ctrl+Shift+R`) to bypass a cached old page.

### Google origin errors

The OAuth client must list exactly:

```text
http://localhost:8080
```

Scheme, host, port, and no trailing slash. `http://127.0.0.1:8080` is a different
origin; use the same host in the browser and in the console. If you serve on a
different port, register that origin too.

### Popup blocked

Google One Tap may be blocked by the browser or an extension. Allow popups for
`localhost:8080`, disable strict tracking protection for the site, and retry. A
private window with third-party cookies blocked can also break the flow.

### “Missing ticket”

The order matters:

```text
Google credential (web / Windows helper)
  OR email/password (Android)
  → pre-login returns ticket
  → enter the emailed code
  → email/login or google/login exchanges ticket + code
```

Pressing “send code” alone returns `{status:true}` and no ticket. If the ticket
expired, go back and sign in again to obtain a fresh one.

### CORS errors

The page must be served from an origin that `api.sjrc.club` allows. `file://`
sends `Origin: null` and will fail. If a proxy or extension strips CORS headers,
disable it for the test.

## Authentication and tokens

### `10117` / `10119`

The access token is invalid or expired. The app tries a refresh once. If that
fails:

- Web: log out, clear site data for `localhost:8080`, sign in again.
- Android: app info → clear storage, sign in again.
- Windows: clear the token field and re-import; if refresh is stale, delete the
  registry key and sign in again.

### `20042` / `20043`

`20042` means the refresh token itself expired. `20043` often means it belongs to
a session the server no longer knows. Both require a new sign-in. Clearing only
the access token is not enough.

### `20045`

The account is not registered with SJRC. Check the email, or contact the school.
The client does not retry.

### `20002`

The Google ID token was issued for a different client type or origin. Create a
**Web application** OAuth client, add `http://localhost:8080` as an authorized
JavaScript origin, and use that client ID. An Android client ID will not work in
a browser.

## Protocol errors

### `10102` Bad Request

Most often the heartbeat `time` is milliseconds. Send `3`, not `3000`. Other
causes:

- `sessionId` is missing or is the literal string `null`.
- The body was sent as form data instead of JSON.
- A field name is misspelled; the server validates the DTO.

Check the Swagger document for the exact DTO:
<https://api.sjrc.club/swagger-json>.

### `10104` Forbidden

`x-auth-schema` does not match the route. Public routes (`v1/public/...`) need
`PUBLIC`; client routes need `CLIENT`. A missing schema is also rejected.

### `80009` on end

The end DTO requires `sessionId`. The official client sends `bookId`, which is a
bug for this server version. All builds here send `sessionId`. If you still get
80009, the session was already ended, expired, or belongs to another account.

### `-777777` / `-1001`

`-777777` means remaining quota is at or below 20% of the total. It is a ratio,
not 20 minutes. `-1001` is a forced logout after repeated failures; the client
stops after 4.

## Quota and progress

### Quota does not move

- Confirm demo mode is off.
- Confirm the log shows `POST .../start` with a `sessionId`.
- Confirm the heartbeat request returns `200`/`201`, not an error.
- Check the account card: a different account may be signed in.
- Server statistics can lag; refresh the quota card.

### Progress jumps or is negative

The progress card uses `max(local reported seconds, server used delta)`. A
server-side change (another device, a quota reset) can move the baseline. Stop
and restart to get a fresh baseline.

### Target reached immediately

The target is in hours and must be > 0. `0.1` is six minutes. If the quota delta
is already larger than the target, the app stops on the first tick; start a new
run after checking the quota.

## Android

### “App not installed”

- The APK is signed with a different key than an installed build of the same
  package. Uninstall the old `club.sjrc.autoread` first.
- The system blocks sideloading. Allow unknown sources for the file manager.
- The APK is corrupted. Re-copy and verify SHA-256.

This package is not the official `club.sjrc.app`; it should install alongside it.

### Blank WebView

The checked-in APK enables:

```smali
setAllowFileAccessFromFileURLs(true)
setAllowUniversalAccessFromFileURLs(true)
```

A rebuild that omits them shows a blank page. Also check that
`assets/index.html` exists and `setJavaScriptEnabled(true)` is set.

### Google sign-in missing

Intentional. Google blocks OAuth in embedded WebViews. Use email + password. If
the account has no password, set one in the official app
(**我的 → 密碼 → 修改**).

### Timer stops in the background

Android throttles WebView timers. Keep the app in the foreground, disable battery
optimisation for the app if your device allows it, and expect the session to end
when `onStop()` fires.

## Windows

### SmartScreen / antivirus warning

The binary is not code-signed. Verify:

```bat
certutil -hashfile SJRC_AutoReader.exe SHA256
```

Compare with `SHA256SUMS`. If it matches, choose **More info → Run anyway**. If it
does not, delete it.

### Missing DLL

The official build uses `-static`. If you rebuilt it without `-static` and see a
missing `libstdc++-6.dll` or `libgcc_s_seh-1.dll`, rebuild with:

```bat
g++ ... -static -static-libgcc -static-libstdc++
```

### Local sign-in page does not open

- Port `8080` may be used by another server. Close it or use manual token import.
- Port `47821` may be blocked by a firewall. Allow loopback.
- The helper closes after 2–3 minutes idle; reopen it.

### Token field not importing

Paste only the token value, not the whole JSON, or paste the JSON — the importer
accepts both. If it still fails, use the plain access token. Do not include
quotes or `Bearer ` unless the importer expects it; the checked build strips
common prefixes.

### Registry permission error

The app writes to `HKCU`, which normally needs no elevation. A locked-down
machine may block it. Run as the same user, or use a portable copy and re-enter
settings each time.

## Verification and logs

Before reporting, collect:

```bash
python3 scripts/check_repo.py
sha256sum -c SHA256SUMS
```

and the last 20 log lines with secrets removed. Include:

- build (web/Android/Windows) and version,
- OS/browser version,
- exact error code,
- request path,
- whether demo mode was on.

Do not include tokens, session IDs, emails, school names, or book titles. See
[SUPPORT.md](../../SUPPORT.md).
