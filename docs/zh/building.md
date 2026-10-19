# 建置

**繁體中文** · [English](../en/building.md)

## 摘要

| 目標 | 建置系統 | 工具鏈 | 產出 |
|---|---|---|---|
| 網頁 | 無 | 任何文字編輯器 | `web/SJRC_AutoReader.html` |
| Android 外殼 | apktool + apksigner，或小型 Gradle 專案 | Android SDK 33、JDK 17 | 已簽章 APK |
| Windows | `build.bat` | MinGW-w64 g++ + windres | `SJRC_AutoReader.exe` |
| 文件／資料 | Python 腳本 | Python 3.10+ | 已檢查的樹 |

建置已發佈的成品不需要 npm、pip、Maven 或 NuGet 依賴。`phone/src/` 的 Android
原始碼是節錄，不是完整 Gradle 專案；重建方式見下。

## 網頁

成品就是原始碼。直接編輯 `web/SJRC_AutoReader.html`。

```bash
# 檢查 inline script 語法（Node 18+）
python3 - <<'PY'
import re
html = open('web/SJRC_AutoReader.html', encoding='utf-8').read()
js = re.search(r'<script>(.*)</script>', html, re.S).group(1)
open('.web-check.js', 'w', encoding='utf-8').write(js)
PY
node --check .web-check.js
rm .web-check.js

# 本機測試
python3 scripts/serve_web.py
```

沒有書面提案不要引入 bundler 或框架。單檔是安全敘事的一部分：只有一個檔案要審核。

## Android 外殼

### 方案 A —— 修補現有 APK（最快）

已提交的 APK 就是成品。要換嵌入頁面：

```bash
apktool d phone/SJRC-AutoRead.apk -o build/phone
# 用你要的頁面取代 build/phone/assets/index.html
apktool b build/phone -o build/SJRC-AutoRead-unsigned.apk
zipalign -f 4 build/SJRC-AutoRead-unsigned.apk build/SJRC-AutoRead-aligned.apk
apksigner sign --ks release.jks --out build/SJRC-AutoRead.apk \
  build/SJRC-AutoRead-aligned.apk
apksigner verify --print-certs build/SJRC-AutoRead.apk
```

`apktool`、`zipalign`、`apksigner` 來自 Android SDK build-tools。簽章金鑰不要
提交到儲存庫。

### 方案 B —— 重建最小外殼

要乾淨的原始碼建置，建立小型 Android 專案：

```text
app/src/main/AndroidManifest.xml
app/src/main/assets/index.html
app/src/main/java/club/sjrc/auto/MainActivity.java
app/build.gradle
```

Manifest 重點：

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

Activity 重點：

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

Manifest 的完整類別名必須與 Java package 一致。簡寫 `.MainActivity` 會展開成
`<package>.MainActivity`，這是「class not found」的常見原因。

建置與簽章：

```bash
./gradlew assembleRelease
apksigner sign --ks release.jks app/build/outputs/apk/release/app-release-unsigned.apk
```

### 方案 C —— 完全不用 SDK

已提交的 APK 是透過修補最小 binary manifest 並插入 dex/assets 產生的。這條路
很脆弱，不建議新貢獻者使用。有 SDK 就用方案 A 或 B。

## Windows

安裝 MinGW-w64（MSYS2：`pacman -S mingw-w64-x86_64-gcc`），或在 Linux 用交叉
編譯器：

```bash
# 在 pc/src
windres app.rc -O coff -o app.res.o
g++ -O2 -std=c++17 -municode main.cpp app.res.o -o SJRC_AutoReader.exe \
    -static -static-libgcc -static-libstdc++ \
    -lwinhttp -lcomctl32 -lws2_32 -lole32 -luser32 -lgdi32 -lshell32
```

或在 MinGW shell 執行已提交的批次檔：

```bat
cd pc\src
build.bat
```

注意：

- 入口是 `wWinMain`，所以需要 `-municode`。
- `-static` 移除 `libstdc++-6.dll` 與 `libgcc_s_seh-1.dll` 依賴。
- 目標是 GUI subsystem，不會跳出 console。
- `app.ico` 經 `app.rc` 嵌入；兩個檔案要放在一起。
- 從 Linux 交叉編譯可用 `x86_64-w64-mingw32-g++` 與
  `x86_64-w64-mingw32-windres`，旗標相同。

## 可重現性

二進位檔已提交，使用者不需要工具鏈。重建不會 byte-identical，因為時間戳與
編譯器版本不同。重要的是：

- `SHA256SUMS` 與提交的二進位檔相符。
- `data/source-stats.json` 記錄大小與雜湊。
- 重建的二進位檔在目標平台測試過才替換。

發佈時，把重建的二進位檔與雜湊附在 GitHub release；不要在同一個 commit 裡
無聲覆蓋已提交的檔案又改原始碼。

## 建置後檢查

```bash
python3 scripts/check_repo.py
python3 scripts/package_release.py
```

只有在成品變更時才更新 `SHA256SUMS` 與 `data/source-stats.json`，並在同一個
pull request 完成，讓審查者同時看到兩者。
