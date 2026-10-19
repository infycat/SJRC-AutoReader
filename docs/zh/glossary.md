# 名詞解釋

[English](../en/glossary.md) · 繁體中文

名詞依出現場合分組。「用於」欄指向建議先讀的文件，不是完整索引。

## 核心協議

| 名詞 | 意思 | 用於 |
|---|---|---|
| **心跳（heartbeat）** | 週期性的 `PATCH /v1/client/reading-time`，告訴伺服器「這個 session 還在讀」。官方客戶端每 3 秒送一次，`time: 3`。 | [API](api.md) |
| **閱讀 session** | `reading-time/start` 在伺服器建立的物件，以 `sessionId` 識別。心跳、位置、額度與 `end` 都指向它。 | [使用](usage.md) |
| **`sessionId`** | `start` 回傳的不透明識別碼。心跳與 `end` 都需要它；**不是** `bookId`。 | [API](api.md#本專案使用的端點) |
| **`bookId`** | 書籍識別碼。用於 `start`、`books/{id}`、`reading-pos`；官方客戶端誤把它送給 `end`，產生 `80009`。 | [疑難排解](troubleshooting.md#end-回-80009) |
| **閱讀位置** | 翻頁時由 `POST /v1/client/reading-pos` 送出的 `{chapterId, chapterLocation, position}`。 | [API](api.md#閱讀位置-payload) |
| **額度（quota）** | 伺服器追蹤的時間預算 `totalQuota` / `usedQuota`，單位是**秒**。`28800` 是 8 小時。 | [API](api.md#單位) |
| **目標時數** | 使用者輸入的目標，單位小時。客戶端會換成秒再與額度比較。 | [設定](configuration.md) |
| **`x-auth-schema`** | 選擇認證契約的標頭：已認證客戶端路由用 `CLIENT`，登入／續期路由用 `PUBLIC`。 | [API](api.md#標頭) |
| **Bearer token** | `accessToken`，只在 schema 為 `CLIENT` 且 token 非空時以 `authorization: Bearer <token>` 送出。 | [安全](security.md) |
| **Refresh token** | 長效 token，在 `token/refresh` 換取新 access token。過期代碼是 `20042`。 | [API](api.md#續期) |
| **Ticket** | `pre-login` 回傳的短效、一次性值。必須與驗證碼一起送到 `email/login` 或 `google/login`；只有驗證碼不夠。 | [API](api.md#認證) |
| **`code` / `isSucc`** | JSON 信封的業務狀態。成功是 `code == 200 || code == 201`；只看 HTTP 狀態不夠。 | [API](api.md#回應信封) |
| **`errorTime`** | 連續心跳失敗次數。官方客戶端用它乘上基礎 `time`，連續四次失敗後強制登出（`-1001`）。 | [逆向分析](reverse-engineering.md#心跳狀態機) |
| **`-777777`** | 內部事件，表示「剩餘額度小於等於總額度 20%」。名字是比例檢查，不是 20 分鐘。 | [疑難排解](troubleshooting.md#-777777--1001) |
| **`80008`** | 額度用完的伺服器代碼。停止執行；重試不會生出額度。 | [疑難排解](troubleshooting.md#配額與進度) |
| **`80009`** | 「找不到閱讀 session」。最常見原因是 `end` 送了 `bookId` 而不是 `sessionId`。 | [疑難排解](troubleshooting.md#end-回-80009) |

## 客戶端與成品

| 名詞 | 意思 | 用於 |
|---|---|---|
| **網頁版** | 單一檔案 `web/SJRC_AutoReader.html`。成品即源碼，沒有建置步驟。 | [架構](architecture.md#網頁版) |
| **Android 外殼** | `phone/SJRC_AutoRead.apk`，套件 `club.sjrc.autoread`，包住內嵌網頁的 WebView 外殼。不是官方 App。 | [安裝](installation.md#android) |
| **Windows 版** | `pc/SJRC_AutoReader.exe`，原生 Win32/WinHTTP、靜態連結、設定存登錄檔。 | [架構](architecture.md#windows-原生版) |
| **官方客戶端** | SJRC Android App `club.sjrc.app` 2.9.0（versionCode 120），協議參考對象。 | [逆向分析](reverse-engineering.md#分析對象) |
| **WebView** | Android 的嵌入式瀏覽器元件。外殼使用它；Google OAuth 拒絕在其中執行。 | [疑難排解](troubleshooting.md#沒有-google-登入) |
| **GIS** | Google Identity Services，網頁版用來取得 ID token 的 JavaScript 函式庫。需要註冊 JavaScript origin。 | [設定](configuration.md#網頁與-android-欄位) |
| **模擬模式** | 離線操作 UI。不送任何網路請求，不影響額度。 | [使用](usage.md) |
| **本機登入輔助頁** | Windows 專用的臨時 localhost 頁面，用來取得 token；埠 47821 與 8080。 | [使用](usage.md#windows-本機登入頁) |

## 儲存庫與源碼

| 名詞 | 意思 | 用於 |
|---|---|---|
| **Smali** | DEX 位元碼的可讀反組譯。儲存庫保留 4,888 個類別作為位元級證據。 | [逆向分析](reverse-engineering.md#為什麼以-smali-為準) |
| **APK / APKS** | Android 安裝包／分卷包。官方 2.9.0 是 APKS：base + ABI + 密度 + 語言分卷。 | [逆向分析](reverse-engineering.md#分析對象) |
| **Retrofit** | 官方 App 使用的 HTTP 客戶端函式庫；端點以註解方法宣告。 | [API](api.md#傳輸) |
| **OkHttp** | Retrofit 底層的 HTTP 傳輸。官方設定 60 秒逾時、不跟隨重導、不憑證釘選。 | [逆向分析](reverse-engineering.md#請求管線) |
| **Interceptor** | OkHttp 掛鉤，用來加標頭或續期 token。最重要的是 `ParameterInterceptor` 與 `TokenExpireInterceptor`。 | [逆向分析](reverse-engineering.md#客戶端架構) |
| **BodyBuilder** | 官方 App 的 map-to-JSON 輔助器。沒有型別安全，所以單位或欄位寫錯只會在執行期爆。 | [逆向分析](reverse-engineering.md#兩套網路風格並存) |
| **DTO** | 資料傳輸物件；具型別的請求／回應形狀。`EndReadingDto` 需要 `sessionId`。 | [API](api.md#本專案使用的端點) |
| **DRM** | 數位版權管理。官方客戶端有內容加密層；三個客戶端不實作，公開鏡像也不附可用金鑰。 | [法律](legal.md#不繞過-drm) |
| **Manifest** | `AndroidManifest.xml`，宣告套件、SDK 等級、權限與 activity。兩個 App 的解碼副本都保留。 | [建置](building.md#android-外殼) |
| **Golden file** | 簽入的預期輸出，用來偵測意外變動。端點表與校驗碼可視為輕量 golden file。 | [測試](testing.md) |

## 慣例

- 「官方」一律指 SJRC 2.9.0，不是本儲存庫。
- JSON 裡未標單位的時間一律是**秒**。
- `club.sjrc.app` 是原始套件；`club.sjrc.autoread` 是本專案外殼。
- `→` 表示請求，`←` 表示回應或由回應產生的日誌。
- 未特別註明時，檔案路徑相對於儲存庫根目錄。
