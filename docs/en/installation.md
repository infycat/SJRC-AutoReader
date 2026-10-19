# Installation

[中文](../zh/installation.md) · English

Each platform download is a complete ZIP bundle. There is no installer,
background service, or auto-updater. The web bundle's launcher requires Python 3.

Download the current files from the [latest GitHub Release](https://github.com/infycat/SJRC-AutoReader/releases/latest).

## Choose a build

| | Web | Android | Windows |
|---|---|---|---|
| Download | [`SJRC-AutoReader-Web.zip`](https://github.com/infycat/SJRC-AutoReader/releases/latest/download/SJRC-AutoReader-Web.zip) | [`SJRC-AutoReader-Android.zip`](https://github.com/infycat/SJRC-AutoReader/releases/latest/download/SJRC-AutoReader-Android.zip) | [`SJRC-AutoReader-Windows.zip`](https://github.com/infycat/SJRC-AutoReader/releases/latest/download/SJRC-AutoReader-Windows.zip) |
| Contents | HTML, Python server, Windows `.bat` launcher, guide | APK, guide, licence, checksums | EXE, guide, licence, checksums |
| Install | extract, then run launcher | extract, then sideload APK | extract, then run EXE |
| Google sign-in | ✅ (HTTP origin) | ❌ | ✅ via local helper |
| Email + password | ❌ | ✅ | ❌ |
| Paste access token | ❌ | ❌ | ✅ |
| Background operation | browser tab must stay alive | WebView must stay foreground-ish | native worker |
| Storage | `localStorage` | WebView `localStorage` | `HKCU\Software\SJRC\AutoReader` |

## Verify the download

Download the platform ZIP and its matching `.zip.sha256` sidecar from the
[latest Release](https://github.com/infycat/SJRC-AutoReader/releases/latest). Verify
the ZIP first, extract it, then run the included `sha256sum -c SHA256SUMS` to
verify its contents:

```bash
# Linux / macOS
sha256sum -c SJRC-AutoReader-Web.zip.sha256
shasum -a 256 -c SJRC-AutoReader-Web.zip.sha256  # macOS
sha256sum -c SHA256SUMS  # run after extraction, inside the bundle folder
```

```bat
:: Windows: compare this output with the hash in the matching .zip.sha256 file
certutil -hashfile SJRC-AutoReader-Web.zip SHA256
```

Expected values are also in `data/source-stats.json`. A mismatch means the file
was modified or corrupted; do not run it.

## Web

### Serve over HTTP

The page must have an `http://` origin for Google Identity Services. Do not open
it with `file://`. In the downloaded Web ZIP, double-click
`Start-SJRC-AutoReader.bat`; this starts the included local server and opens the
page. Python 3 must be installed. On macOS/Linux, run `python3 serve_web.py` in
the extracted folder.

```bash
cd web
python3 -m http.server 8080
```

Open <http://localhost:8080/SJRC_AutoReader.html>.

The repository also ships a slightly better server:

```bash
python3 scripts/serve_web.py
```

It binds `127.0.0.1:8080`, sets `no-store` for the app, and prints the exact URL.

### Google sign-in origin

In [Google Cloud Console](https://console.cloud.google.com/apis/credentials) →
your OAuth 2.0 Client ID → **Authorized JavaScript origins**, add:

```text
http://localhost:8080
```

Paste the same Client ID into the app's **Google Client ID** field. If the
origin is missing, the Google button shows an origin error and no token is
issued. Email sign-in does not need this.

### Stop serving

`Ctrl+C` in the terminal. There is nothing installed and nothing to uninstall.

## Android

1. Extract the Android ZIP, then copy `SJRC-AutoRead.apk` to the device (USB, cloud drive, or `adb`).
2. Open it with the file manager.
3. Allow **Install unknown apps** for that file manager when prompted.
4. Install and open **SJRC AutoRead**.
5. Sign in with email + password. Google sign-in is intentionally absent.

Package facts:

| Item | Value |
|---|---|
| Package | `club.sjrc.autoread` |
| Version | `1.0` (versionCode 1) |
| minSdk / targetSdk | 23 / 33 |
| Permissions | `INTERNET`, `ACCESS_NETWORK_STATE` |
| Signature | V1 + V2 + V3 |

This is a **different package** from the official client (`club.sjrc.app`).
Both can be installed side by side. Do not try to install this APK as an update
to the official app; the signatures differ.

### Uninstall

Settings → Apps → **SJRC AutoRead** → Uninstall. This removes the WebView
storage, including any saved token.

## Windows

1. Extract the Windows ZIP to a folder you control.
2. Double-click it. SmartScreen may warn because the binary is not code-signed;
   choose **More info → Run anyway** only if the SHA-256 matches.
3. Sign in with the local helper or paste a token.

No registry entries are created until you save a setting or token. To remove
them manually:

```bat
reg delete "HKCU\Software\SJRC\AutoReader" /f
```

The executable is statically linked. It does not need the Visual C++
redistributable, .NET, Java, or a browser.

## Network requirements

| Destination | Why |
|---|---|
| `https://api.sjrc.club/` | all reading-time, auth, book metadata calls |
| `https://accounts.google.com/` | Google Identity Services (web / helper) |
| `https://www.gstatic.com/` | Google Identity Services script |

No other host is contacted. A corporate proxy that intercepts TLS will break the
app; do not disable certificate validation to work around it.

## Updating

Replace the file with the new release. The web app and Windows build keep their
existing local storage/registry settings. For Android, install the new APK over
the old one **only if it is signed with the same key**; otherwise uninstall first
and expect to sign in again.

## Removing all traces

| Build | Action |
|---|---|
| Web | Delete the HTML file; clear site data for `localhost:8080` |
| Android | Uninstall the app |
| Windows | Delete the EXE; delete `HKCU\Software\SJRC\AutoReader` |

There is no cloud account, no server-side profile, and no analytics identifier
to delete.
