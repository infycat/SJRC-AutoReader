# 安裝

**繁體中文** · [English](../en/installation.md)

每個成品都是單一檔案。沒有安裝器、沒有背景服務、沒有自動更新、沒有執行期要裝。
最新版本請從 [GitHub Releases](https://github.com/infycat/SJRC-AutoReader/releases/latest) 下載。

## 選擇成品

| | 網頁 | Android | Windows |
|---|---|---|---|
| 檔案 | [`SJRC_AutoReader.html`](https://github.com/infycat/SJRC-AutoReader/releases/latest/download/SJRC_AutoReader.html) | [`SJRC-AutoRead.apk`](https://github.com/infycat/SJRC-AutoReader/releases/latest/download/SJRC-AutoRead.apk) | [`SJRC_AutoReader.exe`](https://github.com/infycat/SJRC-AutoReader/releases/latest/download/SJRC_AutoReader.exe) |
| 大小 | 86,464 B | 53,784 B | 991,237 B |
| 安裝 | 無 | 側載 | 無 |
| Google 登入 | ✅（需 HTTP origin） | ❌ | ✅ 經本機登入頁 |
| 電郵 + 密碼 | ❌ | ✅ | ❌ |
| 貼上 access token | ❌ | ❌ | ✅ |
| 背景執行 | 瀏覽器分頁需保持 | WebView 盡量保持前景 | 原生 worker |
| 儲存 | `localStorage` | WebView `localStorage` | `HKCU\Software\SJRC\AutoReader` |

## 驗證下載

從[最新 Release](https://github.com/infycat/SJRC-AutoReader/releases/latest) 下載 `SHA256SUMS`，
與成品放在相同目錄，再執行其中一種：

```bash
# Linux / macOS
sha256sum -c SHA256SUMS
# macOS 沒有 sha256sum 時
shasum -a 256 web/SJRC_AutoReader.html phone/SJRC-AutoRead.apk pc/SJRC_AutoReader.exe
```

```bat
:: Windows
certutil -hashfile web\SJRC_AutoReader.html SHA256
certutil -hashfile phone\SJRC-AutoRead.apk SHA256
certutil -hashfile pc\SJRC_AutoReader.exe SHA256
```

預期值也在 `data/source-stats.json`。雜湊不符代表檔案被修改或損壞，不要執行。

## 網頁

### 用 HTTP 提供

Google Identity Services 需要 `http://` origin，不要用 `file://` 開啟。

```bash
cd web
python3 -m http.server 8080
```

開 <http://localhost:8080/SJRC_AutoReader.html>。

儲存庫也附一個較好的伺服器：

```bash
python3 scripts/serve_web.py
```

它綁定 `127.0.0.1:8080`、對應用設定 `no-store`，並印出確切網址。

### Google 登入 origin

在 [Google Cloud Console](https://console.cloud.google.com/apis/credentials) →
你的 OAuth 2.0 Client ID → **Authorized JavaScript origins** 加入：

```text
http://localhost:8080
```

把同一個 Client ID 貼到應用中的 **Google Client ID** 欄位。若 origin 未登記，
Google 按鈕會顯示 origin 錯誤，不會發出 token。網頁版沒有其他登入方式，所以這一步是必要的。

### 停止伺服器

在終端按 `Ctrl+C`。沒有安裝任何東西，也沒有東西要卸載。

## Android

1. 把 `phone/SJRC-AutoRead.apk` 複製到裝置（USB、雲端硬碟或 `adb`）。
2. 用檔案管理員開啟。
3. 提示時，為該檔案管理員允許**安裝未知應用程式**。
4. 安裝並開啟 **SJRC AutoRead**。
5. 用電郵 + 密碼登入。Google 登入刻意不提供。

套件資訊：

| 項目 | 值 |
|---|---|
| Package | `club.sjrc.autoread` |
| 版本 | `1.0`（versionCode 1） |
| minSdk / targetSdk | 23 / 33 |
| 權限 | `INTERNET`、`ACCESS_NETWORK_STATE` |
| 簽章 | V1 + V2 + V3 |

這與官方客戶端（`club.sjrc.app`）是**不同套件**，兩者可並存。不要把它當成官方
App 的更新來安裝；簽章不同。

### 卸載

設定 → 應用程式 → **SJRC AutoRead** → 解除安裝。這會移除 WebView 儲存，包括
已存的 token。

## Windows

1. 把 `pc/SJRC_AutoReader.exe` 放到你能控制的資料夾。
2. 雙擊執行。因為二進位檔沒有程式碼簽章，SmartScreen 可能警告；只有在 SHA-256
   相符時才選**更多資訊 → 仍要執行**。
3. 用本機登入頁登入，或貼上 token。

在你儲存設定或 token 之前，不會建立任何登錄項目。要手動移除：

```bat
reg delete "HKCU\Software\SJRC\AutoReader" /f
```

執行檔是靜態連結，不需要 Visual C++ 可轉散發套件、.NET、Java 或瀏覽器。

## 網路需求

| 目的地 | 用途 |
|---|---|
| `https://api.sjrc.club/` | 所有閱讀時間、認證、書本 metadata |
| `https://accounts.google.com/` | Google Identity Services（網頁／登入頁） |
| `https://www.gstatic.com/` | Google Identity Services script |

不會連線到其他主機。會攔截 TLS 的企業代理會讓應用失敗；不要為了繞過而停用
憑證驗證。

## 更新

用新版本檔案取代舊檔。網頁版與 Windows 版會保留原本的 localStorage／登錄設定。
Android 只有在**同一把金鑰簽章**時才能覆蓋安裝；否則需先卸載，並預期要重新登入。

## 移除所有痕跡

| 成品 | 動作 |
|---|---|
| 網頁 | 刪除 HTML；清除 `localhost:8080` 的網站資料 |
| Android | 解除安裝 |
| Windows | 刪除 EXE；刪除 `HKCU\Software\SJRC\AutoReader` |

沒有雲端帳號、沒有伺服器端 profile、沒有分析識別碼要刪。
