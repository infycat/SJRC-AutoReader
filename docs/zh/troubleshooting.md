# 疑難排解

**繁體中文** · [English](../en/troubleshooting.md)

先看確切訊息。日誌會顯示請求路徑與 body `code`；code 比 HTTP status 更有用。

## 快速診斷

| 症狀 | 可能原因 | 修法 |
|---|---|---|
| 網頁空白 | 用 `file://` 開啟，或 script 錯誤 | 用 HTTP 提供；開 DevTools |
| Google 按鈕沒反應 | origin 未登記、Client ID 空、彈窗被擋 | 加 `http://localhost:8080`；貼 Client ID |
| `disallowed_useragent` | WebView 內做 Google OAuth | 用電郵密碼（Android 沒有 Google） |
| `Missing ticket` | 沒有 `pre-login` ticket 就輸入驗證碼 | 先做密碼步驟；重新取得驗證碼 |
| `10102` | `time` 送毫秒、缺 `sessionId`、JSON 錯 | 送秒；檢查 body |
| `10104` | `x-auth-schema` 不匹配 | 公開路由 `PUBLIC`；客戶端路由 `CLIENT` |
| `10117` / `10119` | Token 無效／過期 | 續期；失敗就重新登入 |
| `20002` | Google ID token 被拒 | 用 origin `localhost:8080` 的 web client |
| `20042` / `20043` | Refresh 過期／session 過時 | 清儲存，重新登入 |
| `80008` | 配額用盡 | 停止；等新配額週期 |
| `80009` | `end` 送了 `bookId` | 送 `sessionId` |
| 配額不動 | 模擬模式、start 失敗、帳號錯 | 看日誌有沒有 start；確認帳號 |
| 目標立即停止 | 目標已達到或為 0 | 設正數目標；重新開始 |
| Android「App not installed」 | 與官方 App 簽章衝突 | 不同套件；卸載衝突版本 |
| Windows SmartScreen | 未簽章二進位 | 驗 SHA-256 後「仍要執行」 |
| Windows 8080 被佔用 | 其他伺服器／登入頁 | 關掉，或手動導入 token |
| 執行一段時間停止 | 4 次心跳失敗、配額、token | 讀最後 10 行日誌 |

## 網頁

### 空白頁

1. 檢查 URL：`file:///...` 會載入 HTML，但 Google GIS 與部分 fetch 行為會失敗。
   改用：

   ```bash
   cd web
   python3 -m http.server 8080
   ```

2. 開 DevTools → Console。語法錯誤代表檔案被改壞。
3. 看 Network 分頁是否有失敗請求。`net::ERR_CONNECTION_REFUSED` 代表伺服器沒開。
4. 強制重新載入（`Ctrl+Shift+R`）避開舊快取。

### Google origin 錯誤

OAuth client 必須列出完全一致的：

```text
http://localhost:8080
```

Scheme、host、port、結尾不能有斜線。`http://127.0.0.1:8080` 是不同 origin；
瀏覽器與 console 要用同一個 host。換 port 就要登記新 origin。

### 彈窗被擋

Google One Tap 可能被瀏覽器或擴充功能擋住。允許 `localhost:8080` 彈窗、關閉該
網站的嚴格追蹤保護，再試。封鎖第三方 cookie 的隱私視窗也可能讓流程失敗。

### 「Missing ticket」

順序很重要：

```text
Google 憑證（網頁／Windows 輔助頁）
  或 電郵密碼（Android）
  → pre-login 回傳 ticket
  → 輸入寄到電郵的驗證碼
  → email/login 或 google/login 兌換 ticket + code
```

只按「發送驗證碼」會回 `{status:true}`，沒有 ticket。Ticket 過期就回到登入步驟
重新取得。

### CORS 錯誤

頁面必須從 `api.sjrc.club` 允許的 origin 提供。`file://` 送 `Origin: null`，
會失敗。若代理或擴充功能移除 CORS 標頭，測試時先停用。

## 認證與 token

### `10117` / `10119`

Access token 無效或過期。應用會嘗試續期一次。若失敗：

- 網頁：登出、清除 `localhost:8080` 網站資料、重新登入。
- Android：應用程式資訊 → 清除儲存、重新登入。
- Windows：清空 token 欄重新導入；若 refresh 過時，刪登錄鍵再登入。

### `20042` / `20043`

`20042` 是 refresh token 本身過期。`20043` 通常是它屬於伺服器已不認得的
session。兩者都要重新登入。只清 access token 不夠。

### `20045`

帳號未在 SJRC 註冊。檢查電郵或聯絡學校。客戶端不會重試。

### `20002`

Google ID token 是給不同 client 類型或 origin 的。建立 **Web application**
OAuth client，把 `http://localhost:8080` 加為 authorized JavaScript origin，
使用該 client ID。Android client ID 不能在瀏覽器用。

## 協議錯誤

### `10102` Bad Request

最常見是心跳 `time` 送了毫秒。送 `3`，不是 `3000`。其他原因：

- `sessionId` 缺少或是字串 `"null"`。
- Body 用了 form data 而不是 JSON。
- 欄位名拼錯；伺服器會驗證 DTO。

確切 DTO 查 Swagger：<https://api.sjrc.club/swagger-json>。

### `10104` Forbidden

`x-auth-schema` 與路由不符。公開路由（`v1/public/...`）要 `PUBLIC`；客戶端路由
要 `CLIENT`。缺少 schema 也會被拒。

### `end` 回 `80009`

End DTO 需要 `sessionId`。官方客戶端送 `bookId`，對這個伺服器版本是 bug。本專案
全部送 `sessionId`。若仍回 80009，代表 session 已結束、過期，或屬於其他帳號。

### `-777777` / `-1001`

`-777777` 代表剩餘配額 ≤ 總量 20%。它是比例，不是 20 分鐘。`-1001` 是連續失敗
後的強制登出；本專案在第 4 次停止。

## 配額與進度

### 配額不動

- 確認模擬模式已關。
- 確認日誌有 `POST .../start` 與 `sessionId`。
- 確認心跳回 `200`/`201`，不是錯誤。
- 檢查帳號卡：可能登入了別的帳號。
- 伺服器統計可能延遲；重新整理配額卡。

### 進度跳動或變負

進度卡用 `max(本機回報秒數, 伺服器 used 差值)`。伺服器端變更（另一台裝置、
配額重置）會移動基準。停止再開始可取得新基準。

### 目標立即達到

目標單位是小時，必須 > 0。`0.1` 是六分鐘。若配額差值已大於目標，應用會在第一
個 tick 停止；確認配額後重新開始。

## Android

### 「App not installed」

- APK 與已安裝的同套件版本簽章不同。先卸載舊的 `club.sjrc.autoread`。
- 系統阻擋側載。為檔案管理員允許未知來源。
- APK 損壞。重新複製並驗證 SHA-256。

這個套件不是官方 `club.sjrc.app`，應可與它並存。

### WebView 空白

已提交的 APK 啟用：

```smali
setAllowFileAccessFromFileURLs(true)
setAllowUniversalAccessFromFileURLs(true)
```

重建時漏掉就會空白。也要確認 `assets/index.html` 存在且
`setJavaScriptEnabled(true)`。

### 沒有 Google 登入

刻意如此。Google 封鎖嵌入式 WebView 的 OAuth。請用電郵 + 密碼。若帳號沒有
密碼，在官方 App 設定（**我的 → 密碼 → 修改**）。

### 背景一段時間就停

Android 會節流 WebView timer。保持前景、在裝置允許時關閉該應用的電池優化，並
預期 `onStop()` 時 session 結束。

## Windows

### SmartScreen／防毒警告

二進位檔沒有程式碼簽章。驗證：

```bat
certutil -hashfile SJRC_AutoReader.exe SHA256
```

與 `SHA256SUMS` 比對。相符就選**更多資訊 → 仍要執行**；不符就刪除。

### 缺少 DLL

官方建置用 `-static`。若重建時沒加而出現缺少 `libstdc++-6.dll` 或
`libgcc_s_seh-1.dll`，重新建置並加：

```bat
g++ ... -static -static-libgcc -static-libstdc++
```

### 本機登入頁打不開

- `8080` 可能被其他伺服器佔用。關掉，或改用手動 token 導入。
- `47821` 可能被防火牆阻擋。允許 loopback。
- 登入頁閒置 2–3 分鐘會關閉；重新開啟。

### Token 欄位導入失敗

只貼 token 值，或貼整個 JSON——導入器兩者都接受。若仍失敗，用純 access token。
除非導入器預期，不要加引號或 `Bearer `；已提交的版本會去除常見前綴。

### 登錄權限錯誤

應用寫 `HKCU`，通常不需要提權。鎖定的機器可能阻擋。用同一個使用者執行，或每次
手動重輸設定。

## 驗證與日誌

回報前收集：

```bash
python3 scripts/check_repo.py
sha256sum -c SHA256SUMS
```

以及移除機密後的最後 20 行日誌。包含：

- 成品（web/Android/Windows）與版本，
- OS／瀏覽器版本，
- 確切錯誤碼，
- 請求路徑，
- 是否開模擬模式。

不要包含 token、session ID、電郵、學校名稱或書名。見
[SUPPORT.zh.md](../../SUPPORT.zh.md)。
