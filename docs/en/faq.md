# Frequently asked questions

[繁體中文](../zh/faq.md) · English

Short answers with links to the long ones. If an answer depends on your account,
school policy, or platform version, that dependency is called out explicitly.

## Project and scope

### Is this affiliated with SJRC or 金閱閣?

No. This is an independent interoperability research project. It is not endorsed
by, supported by, or connected to SJRC, the school behind the deployment, or the
publisher. The names are used only to describe what the software talks to.

### Is automated reading allowed?

It depends on the platform's terms of service and your institution's rules. In
many cases it is not allowed, and accounts can be limited or closed. The project
is published for personal study and protocol research. Read
[Legal and acceptable use](legal.md) before running it, and ask your school if
you are unsure.

### Does this download or decrypt books?

No. The three clients send reading time and a page position; they do not download
book files, request DRM keys, or store decrypted content. The decompiled source
contains evidence that the official client has a content-encryption layer, but
this project neither implements nor ships key material. See
[Reverse engineering](reverse-engineering.md#content-encryption-described-key-material-redacted).

### Why is decompiled client code in the repository?

The official client is distributed publicly as an Android APK. Decompiled smali
was used to recover header names, units, endpoint shapes, and the heartbeat state
machine. Keeping the excerpts makes those claims auditable and lets another
researcher reproduce them. It is reference material only: no build links it in.
The copyright remains with the original owner; see [NOTICE.md](../../NOTICE.md).

### Is there a backend, proxy, or telemetry service?

No. Every build talks directly to `https://api.sjrc.club/`. There is no server
operated by this project, no analytics SDK, no crash reporter, and no account
collection. A token goes from your device to the API and nowhere else.

### Which build should I use?

Use the web build if you want the easiest audit and a modern browser. Use the
Android build if you need a phone and can use email/password sign-in. Use the
Windows build if you want a native executable with registry-backed settings. The
behavioural contract is the same; only the host shell differs.

## Accounts and sign-in

### Why is Google sign-in missing on Android?

Google blocks OAuth in embedded WebViews (`disallowed_useragent`). The official
app uses native Google Sign-In, which requires the package name and signing
certificate to be registered with SJRC's Google Cloud project. This project does
not have that access. Use email + password on Android, or use the web build.

### Why does the verification code arrive but fail to log in?

The code is not a standalone login. The order is: email/password or Google
credential → `pre-login` returns a `ticket` → enter the code → `email/login` or
`google/login` exchanges `ticket + code` for tokens. `send-verifications` alone
returns `{status:true}` and no ticket, so a code obtained that way cannot be
redeemed. See [API reference](api.md#authentication).

### My account has no password. Can I still use the app?

For the web build you can use Google. For Android email/password sign-in you need
a password; the Windows build uses the local Google helper or a pasted token. Set or reset one in the official app
(**我的 → 密碼 → 修改**) or through the platform's account recovery flow. Do not
send your password to anyone, including in an issue.

### Are tokens sent anywhere except the API?

No. The web build keeps them in `localStorage`; the Android wrapper uses its own
embedded page inside a WebView; the Windows build keeps them in
`HKCU\Software\SJRC\AutoReader`. There is no sync, telemetry, or remote logging.
See [Privacy](privacy.md).

### Can I use the same account on two devices?

The platform may allow one active session per device, or may invalidate earlier
tokens. Running two clients against the same account is untested and can confuse
quota accounting. Use one client at a time and stop the first before starting the
second.

## Running

### How does the app accumulate reading time?

It opens a reading session, sends a heartbeat every three seconds
(`PATCH /v1/client/reading-time` with `time: 3`, in seconds), reports page
positions as it turns pages, and closes the session at the end. The server owns
the quota; the client cannot invent seconds. See
[Usage](usage.md#reading-the-status).

### Why a three-second heartbeat and a twenty-second page turn?

Three seconds is what the official 2.9.0 client uses
(`setCountDownInterval(3000)`), and it sends the same value as `time`. Twenty
seconds is this project's default for page turns; it is configurable because the
right cadence depends on book length and quota. Faster is not better: requests can
fail and the server may treat suspicious traffic differently.

### Why does my quota not move?

Check, in order: demo mode is off; the start request succeeded and a `sessionId`
exists; the heartbeat is actually being sent (log lines with `PATCH` and code
`200`); you are looking at the correct account and quota field; the account is not
already at its quota (`80008`). Restarting the browser without closing the old
session can also leave a stale session. See
[Troubleshooting](troubleshooting.md#quota-does-not-move).

### What happens when the target is reached?

The client finishes the current heartbeat/turn, sends
`PATCH /v1/client/reading-time/end`, clears the active session, and stops the
timer. It does not start another run automatically. If the platform's quota is
exhausted, the run stops with `80008` regardless of the target.

### Is the target in hours or seconds?

The UI takes **hours**. The API takes **seconds**. `2.0` hours becomes
`targetSec = 7200`, and the quota fields are compared in seconds. A quota of
`28800` is 8.0 hours, not 8 minutes. See [Units](api.md#units).

### Why did it stop after four errors?

Four consecutive heartbeat failures trigger a forced-stop path (`-1001` in the
official client's event model). The likely causes are an expired token, a deleted
session, or network loss. Refresh the token by signing in again, then start a new
run. Do not keep retrying the same dead session.

## Builds

### What Android version do I need?

The wrapper APK declares `minSdk 23` (Android 6.0) and `targetSdk 33`. The
original SJRC client is a different app (`club.sjrc.app`, `minSdk 26`). The
wrapper's package is `club.sjrc.autoread`, so it can coexist with the official
app, but signing keys must not conflict with a previously installed copy of the
wrapper.

### Why does Windows warn about the executable?

It is an unsigned, statically linked native binary. SmartScreen and some antivirus
products treat any unsigned executable with suspicion. Verify the SHA-256 against
[`SHA256SUMS`](../../SHA256SUMS), scan it if you wish, and only then choose “Run
anyway”. If your organisation blocks unsigned binaries, use the web build.

### Do I need Node, Python, or a runtime installed?

No for normal use. The web build is a single HTML file; the Android build is an
APK; the Windows build is static. Python 3 is useful only for serving the web page
over HTTP and for the repository checks. Node is needed only for the optional
syntax check in CI.

### Why is the web page blank when I double-click it?

The file is designed to be served over HTTP. `file://` has a `null` origin, which
breaks Google Identity Services and can break `fetch`. Run
`python3 -m http.server 8080` in `web/` and open
`http://localhost:8080/SJRC_AutoReader.html`.

### Can I rebuild the APK without the Android SDK?

The repository documents a patch-based route using `apktool` and `apksigner`, and
a minimal Gradle wrapper route. The checked-in APK is already signed and does not
need a rebuild to run. See [Building](building.md#android-wrapper).

## Troubleshooting and contributing

### What should I include in a bug report?

The build and version, operating system, exact error code/message, the relevant
log lines with tokens and personal data removed, and the smallest reproduction.
Do not paste a token, password, verification code, email, school name, or book
title. Use the issue form and follow [SUPPORT.md](../../SUPPORT.md).

### How do I report a security problem?

Follow [SECURITY.md](../../SECURITY.md). Do not open a public issue with exploit
details, tokens, or personal data. If the report is about a third-party service,
contact that service as well; this project cannot fix their server.

### Can I contribute a translation or a fix?

Yes. Read [CONTRIBUTING.md](../../CONTRIBUTING.md). Documentation changes should
keep the English and Chinese pages structurally aligned; `make check` verifies
links, parity, data files, and accidental secrets. Protocol changes should cite
the exact smali class or the public Swagger operation.

### Why not add a backend to make login easier?

A backend would collect account credentials or tokens, create a single point of
failure, and change the privacy model completely. The project deliberately has no
server. If a future feature needs one, it must be proposed as an explicit,
opt-in design with a threat model, not slipped into the clients.

### How can I verify a release?

Compare the archive and binary hashes with `SHA256SUMS`, inspect the source that
ships beside each binary, and review the release notes and `CHANGELOG.md`. For
high-assurance use, rebuild from source using [Building](building.md) and compare
behaviour, not bytes: different compilers produce different executables.
