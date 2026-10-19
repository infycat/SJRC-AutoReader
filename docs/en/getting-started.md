# Getting started

[中文](../zh/getting-started.md) · English

## What this is

SJRC AutoReader is a small, dependency-free client for the reading-time portion
of the SJRC (金閱閣校園版) API. It reproduces the official app's heartbeat and page
turns so a reading session keeps accumulating time until a target you choose.

It is **not** a reader. It never downloads, decrypts, stores, or displays book
content. It has no server and no telemetry.

> **Before you run it:** automating reading time may violate your school's rules
> or the platform's terms of service. Ask first if you are unsure. See
> [legal.md](legal.md).

## Requirements

| Build | Requirements |
|---|---|
| Web | A modern browser (Chrome, Edge, Firefox, Safari). A local HTTP server for Google sign-in. |
| Android | Android 6.0+ (API 23+). “Install unknown apps” permission. |
| Windows | Windows 10/11 x64. No runtime, no installer. |

You also need an SJRC account and a way to sign in. The web build uses Google
Identity Services; the Android wrapper uses email/password plus a verification
code because Google blocks OAuth in embedded WebViews; the Windows build uses a
temporary local Google helper or a pasted access token.

## Five-minute first run (web)

```bash
cd web
python3 -m http.server 8080
# open http://localhost:8080/SJRC_AutoReader.html
```

1. Sign in with Google. The web build does not have an email/password form.
2. Leave **Target hours** at `2` for a first run.
3. Keep **Heartbeat** at `3` seconds and **Page turn** at `20000` ms.
4. Tick **Demo mode** to watch the loop without sending requests.
5. Untick demo, press **Start**, and watch the log and progress card.

Stop with the **Stop** button. The app sends `PATCH /v1/client/reading-time/end`
before it marks the session closed.

## Five-minute first run (Android)

1. Copy `phone/SJRC-AutoRead.apk` to the device.
2. Allow installation from unknown sources for the file manager.
3. Install, open, and sign in with email + password.
4. Set the target and press start. Keep the app in the foreground; Android may
   throttle background timers.

If the page is blank, the WebView flags are missing from the build; the checked-in
APK has them. See [troubleshooting.md](troubleshooting.md).

## Five-minute first run (Windows)

1. Run `pc/SJRC_AutoReader.exe`.
2. Use **Local sign-in page** for Google, or paste an access token.
3. Set the target in hours, heartbeat in seconds, turn interval in seconds.
4. Press **Start**. Closing the window stops the worker and ends the session.

The executable is statically linked; it needs no Visual C++ redistributable.

## What to check after the first run

- `GET /v1/client/reading-time` shows a `usedQuota` increase roughly equal to
  elapsed time in seconds.
- The log shows one heartbeat per 3 seconds (or your configured interval).
- A page turn produces a `POST /v1/client/reading-pos` line.
- Stopping produces a `PATCH /v1/client/reading-time/end` line with code 200/201.

If `usedQuota` jumps by 60× or you see `10102`, you are sending milliseconds
instead of seconds. Read [api.md](api.md#units).

## Where to go next

- [Installation](installation.md) — checksums, uninstall, platform notes
- [Configuration](configuration.md) — every knob and what it changes
- [Usage](usage.md) — a safe, repeatable run loop
- [Security](security.md) — what the tool can and cannot protect
