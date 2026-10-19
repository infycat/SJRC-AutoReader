# Configuration

[中文](../zh/configuration.md) · English

There is no config file. Every build exposes its settings in the UI and persists
them locally. This page lists every knob, its range, its default, and the storage
key behind it.

## Web and Android fields

| UI field | Internal id | Default | Range | Unit | Effect |
|---|---|---:|---|---|---|
| Target hours | `targetHours` | `2` | ≥ 0.1, step 0.1 | hours | Stops when accumulated time reaches this value |
| Pages per chapter | `pagesPerChapter` | `20` | 1–200 | pages | Estimated pages used to build `reading-pos` |
| Heartbeat | `hbMs` | `3` | 1–600 | **seconds** | Interval between `PATCH /reading-time` calls; the value is also sent as `time` |
| Page turn | `turnMs` | `20000` | 2,000–3,600,000 | **milliseconds** | Interval between page turns and `POST /reading-pos` |
| Demo mode | `demoMode` | off | on/off | — | Runs the UI and counters without sending requests |
| Google Client ID | `clientId` | prefilled public web client | string | — | Web only; must match the origin registered in Google Cloud |
| Login type | `loginType` | `web` | `web` / `android` / `ios` | — | Tells the backend which Google client type the ID token came from; use `web` in a browser |

The labels say seconds for heartbeat and milliseconds for page turn; the code
reads `hbMs` and multiplies by 1000, but sends the raw second value as `time`.
Do not “fix” this by sending `hbMs * 1000` to the API.

### Recommended presets

| Scenario | Target | Heartbeat | Page turn | Notes |
|---|---:|---:|---:|---|
| First demo | 0.1 h | 3 s | 20 s | Tick demo mode; no network |
| Normal | 2 h | 3 s | 20 s | Matches the official 3 s cadence |
| Conservative | 1 h | 3 s | 30 s | Fewer position reports |
| Short test | 0.1 h | 3 s | 5 s | Watch two or three turns, then stop |

## Windows fields

| Field | Registry value | Default | Unit | Effect |
|---|---|---:|---|---|
| 目標時長 | `target` | `2` | hours (0.1 precision) | Target stop |
| 心跳 | `hb` | `3` | seconds | Heartbeat interval and `time` |
| 翻頁 | `turn` | `20` | seconds | Page-turn interval |
| Token | `token` | empty | string | Access token |
| Refresh token | `refresh` | empty | string | Refresh token |

All values live under `HKCU\Software\SJRC\AutoReader` as `REG_SZ`. The UI writes
them when a run starts or a token is imported.

## Hard-coded behaviour

These are deliberately not configurable:

| Constant | Value | Why |
|---|---|---|
| API base | `https://api.sjrc.club/` | Only supported service |
| Client version | `2.9.0` | Must match the client version the API expects |
| Locale header | `zh-HK` | Matches the captured client |
| Platform header | `android` | Matches the captured client |
| API timeout (web) | 30,000 ms | Avoids a hung request |
| API timeout (Windows) | 8 s connect, 15 s receive | WinHTTP phase timeouts |
| Token refresh lead | 90 s before JWT expiry | Avoid a heartbeat 401 |
| Windows refresh cadence | every 150 s | Keeps the server session warm |
| Consecutive error limit | 3 (stop on the 4th) | Mirrors the official forced logout |
| Quota warning | remaining ≤ 20% of total | Official ratio, not 20 minutes |
| Log retention | 400 lines (web) | Keeps the DOM small |
| UI tick | 1 s | Single scheduler |

## Storage

### Web / Android

| Key | Storage | Contents | Cleared by |
|---|---|---|---|
| `sjrc.token` | `localStorage` | access token | logout, manual site-data clear |
| `sjrc.refresh` | `localStorage` | refresh token | logout, manual site-data clear |
| `sjrc.ticket` | `sessionStorage` | one-time ticket | code exchange, tab close |
| `sjrc.session` | `localStorage` | `{sessionId, bookId}` | end, logout, page boot cleanup |
| Google Client ID | `localStorage` | public OAuth client id | manual clear |

The access token is a bearer credential. Anyone with access to the browser
profile can read it. Use a dedicated browser profile if that matters to you.

### Windows

Everything is under `HKCU\Software\SJRC\AutoReader`. The token is plain `REG_SZ`.
Remove it with:

```bat
reg delete "HKCU\Software\SJRC\AutoReader" /f
```

## Resetting

| Goal | Web | Android | Windows |
|---|---|---|---|
| Sign out | Logout button | Logout button | Clear the token field / delete registry |
| Forget settings | Clear site data | App info → Clear storage | Delete the registry key |
| Remove everything | Delete file + site data | Uninstall | Delete EXE + registry key |

## Changing the API base

Do not. The project is built for `api.sjrc.club` and a specific client version.
A fork that points at another service is a different project; update the docs,
the security notes, and the tests accordingly.

## A note on `pagesPerChapter`

It does not affect reading time. It only shapes the `reading-pos` payload:
chapter index, page inside chapter, and a `position` ratio. A value that is too
large makes the app look like it is turning pages in a very long chapter; a value
that is too small advances chapters quickly. The default `20` is a reasonable
estimate, not a measurement of the real book.
