# 程式碼地圖

[English](../en/code-map.md) · 繁體中文

每個行為住在哪裡、哪些東西必須一起改、哪些不變量最容易弄壞。行號對應 1.0.0
版簽入的檔案；行號漂移時，函式名與表格才是穩定介面。

> 單一文件的敘事版在 [`CODE-OVERVIEW.md`](../../CODE-OVERVIEW.md)。本頁是任務導向
> 索引：「我想改 X，該開哪些檔案？」

## 儲存庫一眼看

| 路徑 | 角色 | 進入點 |
|---|---|---|
| `web/SJRC_AutoReader.html` | 完整網頁成品。HTML、CSS、JS、圖示都在一個檔案。 | 361 行的 `api()`、1065 行的 `startRun()` |
| `phone/SJRC_AutoRead.apk` | 已簽章 WebView 外殼，套件 `club.sjrc.autoread`。 | `club.sjrc.auto.MainActivity` |
| `phone/src/index.html` | 嵌進 APK 的頁面；多了電郵密碼登入與 `__appPause`，移除僅瀏覽器可用的 Google 流程。 | 1302 行 `emailPreLogin()`、1336 行 `emailLoginByCode()` |
| `phone/src/*.smali` | 六個官方客戶端節錄，作為協議證據。 | `BookReadViewModel`、`ParameterInterceptor` 等 |
| `pc/src/main.cpp` | 整個原生 Windows 客戶端。 | `wWinMain`、1404 行 `startRun()`、806 行 `workerLoop()` |
| 上游 `smali/` 路徑（未隨庫提供） | 研究所用的官方 2.9.0 參考，共 4,888 類；此數字僅為研究統計。 | `club/sjrc/app/net/`、`.../book/viewModel/` |
| `data/` | 正規化端點表與建置統計。 | `api-endpoints.{csv,json}`、`source-stats.json` |
| `docs/en`、`docs/zh` | 雙語使用者與開發文件。 | `docs/README.md` |
| `scripts/` | 離線檢查器、可重現打包器、本機網頁伺服器。 | `check_repo.py`、`package_release.py` |

## 共用契約

三個客戶端沒有共用程式碼，所以這些值重複存在，必須一起改。檢查器能驗證文件與
資料檔和成品雜湊一致，但看不到程式內部的語意漂移。

| 契約項目 | 值 | 網頁 | Windows | Android |
|---|---|---|---|---|
| API base | `https://api.sjrc.club/` | `BASE`，293 行 | `API_HOST` | 內嵌頁面 |
| 版本標頭 | `2.9.0` | `VER` | `API_VER` | 內嵌頁面 |
| 平台標頭 | `android` | `api()` | `httpCall()` | 內嵌頁面 |
| 語系標頭 | `zh-HK` | `api()` | `httpCall()` | 內嵌頁面 |
| 認證 schema | `CLIENT` / `PUBLIC` | 每次呼叫指定 | 每次呼叫指定 | 內嵌頁面 |
| 心跳 | 3 秒，`time: 3` 秒 | `heartbeat()`，889 行 | `workerLoop()`，806 行 | 內嵌頁面 |
| 翻頁 | 預設 20 秒 | `turnPage()`，1024 行 | `workerLoop()`，806 行 | 內嵌頁面 |
| 結束 session | `end {sessionId}` | `endBook()`，872 行 | `endSession()`，613 行 | 內嵌頁面 |
| Token 儲存 | 本機 | `localStorage` | `HKCU\Software\SJRC\AutoReader` | WebView storage |
| User agent | 瀏覽器／預設 | 瀏覽器 | WinHTTP 預設 | WebView 預設 |

Android 資產是網頁版的近似副本。若改了 `web/SJRC_AutoReader.html` 的協議邏輯，
要跟 `phone/src/index.html` 比對，決定手機變體是否也該改。沒有自動同步。

## 網頁版

`web/SJRC_AutoReader.html` 的結構是一個 `<style>`、一棵 UI、一個 `<script>`。
以下都在 script 內。

### 狀態與格式化

| 符號 | 行 | 責任 |
|---|--:|---|
| `BASE`、`VER` | 293 | API 來源與版本標頭 |
| `S` | 296 | 所有可變執行期狀態：token、書、頁、`sessionId`、計數、模擬旗標、額度快照 |
| `fmtHours()` / `fmtHMS()` | 332 / 336 | 把**秒**格式化成小時或 `h m s` |
| `log()` | 343 | 加時間戳記，日誌面板上限 400 行 |
| `netPill()` | 354 | 日誌卡上的網路指示器 |
| `isSucc()` / `codeOf()` / `msgOf()` | 398–400 | 信封輔助；成功是 200 或 201 |

`S` 混了持久值（token、refresh token、session 復原）與衍生計數。新增欄位時要
決定是否跨重新載入存活；要的話，明確加上儲存讀寫與清理路徑。不要把整個 `S`
序列化，裡面有 timer 與 DOM 參照。

### HTTP 與認證

| 符號 | 行 | 責任 |
|---|--:|---|
| `api()` | 361 | 加標頭、`CLIENT` 時加 Bearer、用 `AbortController` 逾時、解析 JSON、更新網路指示 |
| `refreshAccessToken()` | 403 | `POST token/refresh`；更新兩個 token；不強制重試迴圈 |
| `initGsi()` / `tryPromptOneTap()` | 457 / 508 | 載入 Google Identity Services，有 Client ID 時顯示按鈕 |
| `googleSignInClick()` / `onGoogleCredential()` | 524 / 551 | 開帳號選擇器，處理 GIS credential 回呼 |
| `preLogin()` | 591 | `google/pre-login`；直接取得 token 或保存 ticket |
| `adoptLogin()` | 621 | 保存 token、清掉舊 ticket、進入帳號步驟 |
| `doLogin()` | 641 | 用 ticket + 驗證碼呼叫 `google/login` |
| `loadAccount()` / `loadReadingTime()` | 658 / 683 | 讀帳號與額度快照 |

目前**網頁版只提供 Google 登入**；電郵密碼表單只存在於 Android 內嵌頁
（`phone/src/index.html`）。若要在網頁加入電郵登入，請從手機頁移植
`emailPreLogin()`、`emailLoginByCode()`、對應 DOM 與事件，並同步 README 與
登入矩陣，不要只改文件。

必須保留的標頭規則：`version`、`x-current-platform`、`x-current-locale` 永遠
存在；`x-auth-schema` 明確指定；`authorization` **只在** schema 為 `CLIENT` 且
token 非空時附加。公開登入與續期請求不可洩漏舊 Bearer token。

### 書籍與 session 生命週期

| 符號 | 行 | 責任 |
|---|--:|---|
| `fetchCandidateBooks()` | 732 | 先書架、再首頁備援；攤平巢狀書本節點 |
| `fetchBookDetail()` | 758 | `/v2/client/books/{id}`；標題、封面、章節 |
| `startBook()` | 781 | 建立伺服器 session、攤平章節、送初始位置 |
| `sendReadPos()` | 840 | 組 `{bookId, pos:{chapterId, chapterLocation, position}}` |
| `endBook()` | 872 | `PATCH end {sessionId}`，清除 `sjrc.session` |
| `heartbeat()` | 889 | `PATCH reading-time {sessionId, time}`，`time` 單位秒 |
| `onHeartbeatError()` | 942 | 10117/10119 走續期；其他錯誤計數；四次後停止 |
| `updateProgress()` | 968 | 用伺服器差值校正本機已回報秒數 |

`startBook()` 是唯一可以設定 `S.sessionId` 的地方；`endBook()` 是啟動成功後唯一
可以清除它的地方。若新增別的路徑，讓它也走這兩個函式，否則 session 記帳會分叉。

### 執行迴圈與 UI

| 符號 | 行 | 責任 |
|---|--:|---|
| `tick()` / `tickInner()` | 991 / 998 | 一秒驅動；防重入；排心跳與翻頁 |
| `turnPage()` | 1024 | 推進頁面；書末隨機換書或結束 |
| `nextBook()` | 1043 | 選下一本，避免與目前書相同 |
| `startRun()` | 1065 | 驗證目標與 token、重置計數、啟動 timer |
| `stopAll()` | 1112 | 幂等停止：清 timer、恢復按鈕、呼叫 `endBook()` |
| `renderPage()` | 860 | 頁／章計數與進度條 |
| storage recovery | 1179 | 載入時結束 `localStorage` 留下的逾時 session |

`tick()` 用 `tickBusy` 防重入。不要拿掉：心跳會等網路，單純 `setInterval` 在 API
變慢時會疊請求。邏輯順序是：目標檢查 → 心跳到期 → 翻頁到期 → 更新進度。

### 儲存鍵

| 鍵 | 儲存 | 內容 | 清除時機 |
|---|---|---|---|
| `sjrc.token` | `localStorage` | Access token | 登出、手動清除、瀏覽器資料清除 |
| `sjrc.refresh` | `localStorage` | Refresh token（伺服器有回才存） | 登出、續期失敗、手動清除 |
| `sjrc.ticket` | `sessionStorage` | 一次性登入 ticket | 兌換成功、關閉分頁 |
| `sjrc.session` | `localStorage` | `{sessionId, bookId}` 供崩潰復原 | `end` 成功、登出、復原流程 |

## Android 外殼

簽入的 APK 含六個類別：`MainActivity`、`OAuthClient`、`PopupChrome`、`Poster`、
`Probe`、`WebServer`。原始碼樹只有 manifest 與 `MainActivity` 節錄；完整外殼請
反組譯 `classes.dex`。

| 項目 | 事實 |
|---|---|
| 套件 | `club.sjrc.autoread`（與官方 `club.sjrc.app` 不同） |
| SDK | `minSdk 23`、`targetSdk 33` |
| 權限 | `INTERNET`、`ACCESS_NETWORK_STATE` |
| 啟動 activity | `club.sjrc.auto.MainActivity`（manifest 用完整類名） |
| 內容 | `file:///android_asset/index.html` |
| WebView 旗標 | 開啟 file URL 存取與通用存取，頁面才能呼叫 HTTPS |
| 生命週期 | `onStop` 呼叫 JS `__appPause()`，該路徑會結束 session |
| 簽章 | V1 + V2 + V3；APK 已簽章 |
| 登入 | 電郵 + 密碼；WebView 內 Google OAuth 被封鎖 |

外殼原始碼刻意只放部分。重建代表要重做六個類別或換掉外殼；見
[建置](building.md#android-外殼)。內嵌頁才是產品邏輯，外殼只負責承載與轉發暫停事件。

## Windows 版

`pc/src/main.cpp` 是單一翻譯單元，內含小型 JSON 解析器、WinHTTP 客戶端、UI、
登錄儲存、登入輔助服務與工作執行緒。

| 子系統 | 行 | 備註 |
|---|--:|---|
| 常數、顏色、控制項 ID | 1–80 | `API_HOST`、`API_VER`、UI 色盤、`WM_APP_*` |
| JSON 值與解析器 | 116–262 | 極簡遞迴下降；深度上限 64；無外部依賴 |
| `httpCall()` | 276 | WinHTTP：60 秒逾時、不跟隨重導、TLS、標頭、body |
| `api()` 包裝 | 約 414 | 解析 JSON、寫日誌、判斷成功 |
| 登錄 | 385–408 | `HKCU\Software\SJRC\AutoReader`；`target`、`hb`、`turn`、`token`、`refresh` |
| 續期 | 439 | `POST token/refresh`，成功後寫回登錄 |
| `sendReadPos()` | 594 | 閱讀位置 payload |
| `endSession()` | 613 | `PATCH end {sessionId}` |
| 工作狀態 | 340–380 | `Snap` 由 `g_mtx` 保護；計數與額度 |
| `workerLoop()` | 806 | 開書、心跳、翻頁、額度與錯誤停止條件 |
| 本機登入服務 | 約 1466 | 埠 47821/8080；閒置關閉；ticket／驗證碼交換 |
| `startRun()` / `stopRun()` | 1404 / 1440 | 驗證 UI、快照設定、join worker |
| 視窗程序 | 約 2300 | 命令、繪製、timer、關閉處理 |

執行緒規則：UI 狀態放在 `Snap`，在 `g_mtx` 下複製；worker 不可碰 HWND，只能
post `WM_APP_STATE`，並透過封送輔助寫日誌。`WM_CLOSE` 會要求 worker 停止，等
`endSession()` 完成才銷毀視窗。

## 參考源碼（smali）

以下路徑描述上游研究參考中的位置，並非本儲存庫實際檔案；整理後的端點資料與
分析紀錄位於 `data/`。

| 路徑 | 內容 |
|---|---|
| `sjrc-source/smali/club/sjrc/app/net/MainHttpService.smali` | Retrofit 端點宣告，含 `reading-time`、`reading-pos`、`books`、`bookshelf` |
| `.../net/interceptor/ParameterInterceptor.smali` | 標頭注入：版本、語系、平台、`x-auth-schema`、Bearer token |
| `.../net/interceptor/TokenExpireInterceptor.smali` | token 錯誤時的續期與重試 |
| `.../business/book/viewModel/BookReadViewModel.smali` | 閱讀 session、3 秒倒數、`time` 計算、額度判斷、強制登出 |
| `.../manager/TokenManager.smali` | token 保存與清除 |
| `sjrc-source/smali/com/middle/network/BodyBuilder.smali` | map 式請求 body（無型別安全） |
| `sjrc-source/smali/com/middle/network/NetResponse.smali` | 信封與 `isSucc()` 語意 |
| `sjrc-source/smali/com/skytree/epub/` | 分頁引擎；頁數在這裡算，不是伺服器 |

端點字串用 `grep -rn` 找：它們在 DEX 常數池，可能出現在 service 介面而非方法
體內。`data/` 的端點表是正規化萃取，不是手寫猜測；改表時在 `class` 欄記錄來源
方法。

## 資料與產生檔

| 檔案 | 形狀 | 真相來源 |
|---|---|---|
| `data/api-endpoints.csv` | 98 列：`http,path,method,class,description` | 上游 2.9.0 註解（來源樹不隨庫提供） |
| `data/api-endpoints.json` | 98 個方法、67 個唯一 method+path | 同上 |
| `data/api-endpoints-by-module.md` | 依模組分組的摘要 | 由 JSON 產生 |
| `data/source-stats.json` | 版本、SDK、上游統計、本庫來源收錄量、成品雜湊 | 研究紀錄與發行二進位 |
| `SHA256SUMS` | 下載雜湊 | 實際出貨成品 |

JSON 對執行期才給 URL 的方法使用合成路徑 `(dynamic @Url)`。重新產生時保留這個
慣例，否則列數與「唯一路徑」數會悄悄改變。

## 常見修改點

| 目標 | 打開 | 同時檢查 |
|---|---|---|
| 改心跳間隔 | `web/...:889`、`pc/src/main.cpp:806` | `phone/src/index.html`、設定文件、官方 3 秒證據 |
| 新增端點 | `data/api-endpoints.*`、網頁 `api()` 呼叫點、PC `api()` 包裝 | API 文件、`MainHttpService.smali`、錯誤處理 |
| 改預設目標 | 網頁表單預設值、PC `startRun()` 備援 | 開始執行文件、截圖、設定表 |
| 改 token 儲存 | 網頁 `localStorage` 鍵；PC 登錄輔助 | 隱私／安全文件、登出與復原路徑 |
| 新增停止條件 | 網頁 `tick()`／`onHeartbeatError()`；PC `workerLoop()` | 使用文件、錯誤表、測試 |
| 調頁面大小 | 表單的 `pagesPerChapter` | `startBook()`、位置數學、README 警告 |
| 新增語言 | 建立成對的 `docs/en/X.md` 與 `docs/zh/X.md` | `docs/README.md`、檢查器對齊規則、README 索引 |
| 改外殼 | APK manifest 與 `MainActivity` | 手機資產差異、簽章、生命週期暫停 |

## 不變量與陷阱

1. **秒，不是毫秒。** `time: 3`、`totalQuota: 28800`。送毫秒會得到 `10102` 或
   荒謬的進度。
2. **心跳與 end 用 `sessionId`，不是 `bookId`。** 官方客戶端的 `end` 是錯的，
   不要照抄。
3. **每個客戶端只有一個執行迴圈。** 網頁靠 `tickBusy`，Windows 靠
   `g_worker.joinable()`。兩個迴圈會重複計數。
4. **Bearer 只配 `CLIENT`。** 對 `PUBLIC` 路由送 token、或漏掉
   `x-auth-schema`，會得到 `10104`。
5. **Ticket 是一次性的。** 驗證碼交換失敗後不要重試同一張票；重新 pre-login。
6. **Google 不能在 Android WebView 內跑。** 不要用「把 GIS 載進外殼」來修缺少的按鈕。
7. **沒有可用金鑰、沒有內容。** DRM 層只作研究描述；不要把可用的解密程式或金鑰素材加進儲存庫。
