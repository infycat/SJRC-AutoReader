# Windows source

Source of `../../pc/SJRC_AutoReader.exe` — native C++ / Win32, no runtime needed.

| File | Size | Note |
|---|--:|---|
| `main.cpp` | 98 KB | Entire app: Win32 window, WinHTTP client, heartbeat thread |
| `build.bat` | 365 B | One-click build |
| `app.rc` | 39 B | Resource script (icon, version info) |
| `app.ico` | 23 KB | Application icon |

## Build

Needs MinGW-w64 or MSVC.

```bat
build.bat
```

Equivalent command:

```bat
windres app.rc -O coff -o app.res.o
g++ -O2 -std=c++17 -municode main.cpp app.res.o ^
    -o SJRC_AutoReader.exe ^
    -static -static-libgcc -static-libstdc++ ^
    -lwinhttp -lcomctl32 -lws2_32 -lole32 -luser32 -lgdi32 -lshell32
```

`-static` is mandatory — without it the binary needs `libstdc++-6.dll`
and `libgcc_s_seh-1.dll`, which most users do not have.

## Design notes

- **Heartbeat on a worker thread** — the UI never blocks; `WM_CLOSE`
  sends `PATCH /reading-time/end` before exiting.
- **WinHTTP, not WinINet** — not affected by IE settings or system proxy.
- **Settings in the registry** (`HKCU\Software\SJRC\AutoReader`) — no config files.
- **Google helper or manual token** — the app can start a temporary local page
  (ports 47821 and 8080) for Google sign-in, or import a token pasted by the
  user. There is no email/password form in the native client.

## Layout

```text
┌──────────────────────────────────────────┐
│  Status bar: account · quota · elapsed   │
├──────────────────────────────────────────┤
│  [Target hours]  [Start]  [Stop]         │
├──────────────────────────────────────────┤
│  Log: heartbeat #12 ok · page 14/320      │
└──────────────────────────────────────────┘
```
