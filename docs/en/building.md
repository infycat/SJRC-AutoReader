# Building

[中文](../zh/building.md) · English

## Summary

| Target | Build system | Toolchain | Output |
|---|---|---|---|
| Web | none | any text editor | `web/SJRC_AutoReader.html` |
| Android wrapper | apktool + apksigner, or a small Gradle project | Android SDK 33, JDK 17 | signed APK |
| Windows | `build.bat` | MinGW-w64 g++ + windres | `SJRC_AutoReader.exe` |
| Docs/data | Python scripts | Python 3.10+ | checked tree |

There are no npm, pip, Maven, or NuGet dependencies required to build the shipped
artifacts. The Android source in `phone/src/` is an excerpt, not a complete Gradle
project; see the rebuild paths below.

## Web

The product is the source. Edit `web/SJRC_AutoReader.html` directly.

```bash
# syntax check the inline script (Node 18+)
python3 - <<'PY'
import re
html = open('web/SJRC_AutoReader.html', encoding='utf-8').read()
js = re.search(r'<script>(.*)</script>', html, re.S).group(1)
open('.web-check.js', 'w', encoding='utf-8').write(js)
PY
node --check .web-check.js
rm .web-check.js

# serve for manual testing
python3 scripts/serve_web.py
```

Do not introduce a bundler or a framework without a written proposal. The
single-file property is part of the security story: one file to audit.

## Android wrapper

### Option A — patch the existing APK (fastest)

The checked-in APK is the artifact. To change the embedded page:

```bash
apktool d phone/SJRC-AutoRead.apk -o build/phone
# replace build/phone/assets/index.html with the page you want
apktool b build/phone -o build/SJRC-AutoRead-unsigned.apk
zipalign -f 4 build/SJRC-AutoRead-unsigned.apk build/SJRC-AutoRead-aligned.apk
apksigner sign --ks release.jks --out build/SJRC-AutoRead.apk \
  build/SJRC-AutoRead-aligned.apk
apksigner verify --print-certs build/SJRC-AutoRead.apk
```

`apktool`, `zipalign`, and `apksigner` come from the Android SDK build-tools.
Keep the signing key out of the repository.

### Option B — rebuild a minimal wrapper

If you want a clean source build, create a small Android project:

```text
app/src/main/AndroidManifest.xml
app/src/main/assets/index.html
app/src/main/java/club/sjrc/auto/MainActivity.java
app/build.gradle
```

Manifest essentials:

```xml
<manifest package="club.sjrc.autoread">
  <uses-sdk android:minSdkVersion="23" android:targetSdkVersion="33" />
  <uses-permission android:name="android.permission.INTERNET" />
  <uses-permission android:name="android.permission.ACCESS_NETWORK_STATE" />
  <application android:label="SJRC AutoRead" android:usesCleartextTraffic="true">
    <activity android:name="club.sjrc.auto.MainActivity"
              android:exported="true">
      <intent-filter>
        <action android:name="android.intent.action.MAIN" />
        <category android:name="android.intent.category.LAUNCHER" />
      </intent-filter>
    </activity>
  </application>
</manifest>
```

Activity essentials:

```java
WebView web = new WebView(this);
WebSettings s = web.getSettings();
s.setJavaScriptEnabled(true);
s.setDomStorageEnabled(true);
s.setAllowFileAccess(true);
s.setAllowFileAccessFromFileURLs(true);
s.setAllowUniversalAccessFromFileURLs(true);
web.loadUrl("file:///android_asset/index.html");
setContentView(web);
```

The full-qualified class name in the manifest must match the Java package. A
shorthand `.MainActivity` expands to `<package>.MainActivity`, which is a common
cause of “class not found”.

Build and sign:

```bash
./gradlew assembleRelease
apksigner sign --ks release.jks app/build/outputs/apk/release/app-release-unsigned.apk
```

### Option C — no SDK at all

The checked-in APK was produced by patching a minimal binary manifest and
inserting the dex/assets. That path is fragile and not recommended for new
contributors. Use Option A or B if you have an SDK.

## Windows

Install MinGW-w64 (MSYS2: `pacman -S mingw-w64-x86_64-gcc`) or use a
cross-compiler on Linux:

```bash
# from pc/src
windres app.rc -O coff -o app.res.o
g++ -O2 -std=c++17 -municode main.cpp app.res.o -o SJRC_AutoReader.exe \
    -static -static-libgcc -static-libstdc++ \
    -lwinhttp -lcomctl32 -lws2_32 -lole32 -luser32 -lgdi32 -lshell32
```

Or run the checked-in batch file from a MinGW shell:

```bat
cd pc\src
build.bat
```

Notes:

- `-municode` is required because the entry point is `wWinMain`.
- `-static` removes the `libstdc++-6.dll` and `libgcc_s_seh-1.dll` dependency.
- The target is a GUI subsystem app; no console window appears.
- `app.ico` is embedded through `app.rc`; keep both files together.
- Cross-compiling from Linux works with `x86_64-w64-mingw32-g++` and
  `x86_64-w64-mingw32-windres`; use the same flags.

## Reproducibility

The binaries are committed so users do not need a toolchain. Rebuilds will not
be byte-identical because timestamps and compiler versions differ. What matters:

- `SHA256SUMS` matches the committed binary.
- `data/source-stats.json` records the size and hash.
- A rebuilt binary is tested on the target platform before replacing one.

For a release, attach rebuilt binaries and their hashes to the GitHub release;
do not silently overwrite the committed files in the same commit that changes
the source.

## Checks after building

```bash
python3 scripts/check_repo.py
python3 scripts/package_release.py
```

Update `SHA256SUMS` and `data/source-stats.json` only when the artifact changes,
and do it in the same pull request so reviewers can see both.
