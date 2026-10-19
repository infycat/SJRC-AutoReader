# Installation

[中文](../zh/installation.md) · English

Every build is a single file. There is no installer, no background service, no
auto-update, and no runtime to install.

## Choose a build

| | Web | Android | Windows |
|---|---|---|---|
| File | `web/SJRC_AutoReader.html` | `phone/SJRC-AutoRead.apk` | `pc/SJRC_AutoReader.exe` |
| Size | 86,464 B | 53,784 B | 991,237 B |
| Install | none | sideload | none |
| Google sign-in | ✅ (HTTP origin) | ❌ | ✅ via local helper |
| Email + password | ❌ | ✅ | ❌ |
| Paste access token | ❌ | ❌ | ✅ |
| Background operation | browser tab must stay alive | WebView must stay foreground-ish | native worker |
| Storage | `localStorage` | WebView `localStorage` | `HKCU\Software\SJRC\AutoReader` |

## Verify the download

`SHA256SUMS` contains the expected hashes. Run one of:

```bash
# Linux / macOS
sha256sum -c SHA256SUMS
# macOS (no sha256sum)
shasum -a 256 web/SJRC_AutoReader.html phone/SJRC-AutoRead.apk pc/SJRC_AutoReader.exe
```

```bat
:: Windows
certutil -hashfile web\SJRC_AutoReader.html SHA256
certutil -hashfile phone\SJRC-AutoRead.apk SHA256
certutil -hashfile pc\SJRC_AutoReader.exe SHA256
```

Expected values are also in `data/source-stats.json`. A mismatch means the file
was modified or corrupted; do not run it.

## Web

### Serve over HTTP

The page must have an `http://` origin for Google Identity Services. Do not open
it with `file://`.

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

1. Copy `phone/SJRC-AutoRead.apk` to the device (USB, cloud drive, or `adb`).
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

1. Put `pc/SJRC_AutoReader.exe` in a folder you control.
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
