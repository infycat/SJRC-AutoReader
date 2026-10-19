# 全專案程式碼總覽（單一文件）

> **English abstract.** This is the single-file code map for the whole repository.
> It explains what every component does, where the important logic lives, how the
> three clients talk to the SJRC API, and which behaviours are intentional. The
> detailed reference remains in [`docs/`](docs/README.md), but this document is
> designed to be read end-to-end on its own.
>
> 本文件把整個儲存庫的程式碼、資料流、協議與設計取捨總結成一份可獨立閱讀的文件。
> 程式識別字保留英文；說明使用繁體中文。所有行號以目前 `main` 為準。

## 0. 一句話總結

SJRC AutoReader 是一個**不依賴任何執行期函式庫**的閱讀時間客戶端：它重現官方
App 的 3 秒閱讀心跳（`PATCH /v1/client/reading-time`）、在翻頁時回報閱讀位置
（`POST /v1/client/reading-pos`），並在達到指定時數後結束 session。它有三個成品：

| 成品 | 形式 | 進入點 | 大小 |
|---|---|---|---|
| 網頁 | 單一 HTML，無建置 | `web/SJRC_AutoReader.html` | 86,464 B |
| Android | WebView 外殼 APK | `phone/SJRC-AutoRead.apk` | 53,784 B |
| Windows | 原生 Win32/WinHTTP EXE | `pc/SJRC_AutoReader.exe` | 991,237 B |

它**不做**的事：不下載書籍、不解密內容、不帶 DRM 金鑰、不收集憑證、沒有後端、
沒有遙測。這些界線由程式結構與文件共同保證。

## 1. 儲存庫地圖

```text
SJRC-AutoReader-Repo/
├── web/
│   ├── SJRC_AutoReader.html      ★ 完整網頁應用（HTML + CSS + JS 同一個檔案）
│   └── src/README.md             UI 結構、狀態物件、函式索引
├── phone/
│   ├── SJRC-AutoRead.apk         ★ 已簽章 WebView 外殼（minSdk 23 / target 33）
│   └── src/
│       ├── AndroidManifest.xml   外殼 manifest（package club.sjrc.autoread）
│       ├── index.html            嵌入 APK 的同一份網頁 UI（96,272 B）
│       ├── README.md             WebView 設定與建置注意事項
│       └── *.smali               官方客戶端 6 個關鍵類別節錄（協議參考）
├── pc/
│   ├── SJRC_AutoReader.exe       ★ 原生 Windows 客戶端（PE32+ x86-64 GUI）
│   └── src/
│       ├── main.cpp              全部 Windows 邏輯（約 98 KB）
│       ├── build.bat             windres + g++ 靜態連結
│       ├── app.rc / app.ico      資源與圖示
│       └── README.md             建置、登入、登錄檔說明
├── sjrc-source/
│   ├── smali/                    4,888 個官方客戶端類別的 smali（43 MB；金鑰素材已替換）
│   ├── AndroidManifest.xml       官方 2.9.0 manifest（解碼後文字）
│   ├── net_endpoints.json        Retrofit 註解抽出的 98 個方法
│   └── README.md                 反編譯樹的來源、套件分佈、保真度
├── data/
│   ├── api-endpoints.csv/json    清理後的端點表（98 方法 / 67 唯一路徑）
│   ├── api-endpoints-by-module.md
│   ├── source-stats.json         版本、SDK、檔案數、成品 SHA-256
│   └── README.md                 資料來源與重新產生方式
├── docs/{en,zh}/                 17 篇雙語文件
├── scripts/                      檢查、打包、本機伺服器
├── .github/                      CI、issue 表單、PR 模板
├── README.md / README.zh.md      入口
├── CONTRIBUTING.md / SECURITY.md / SUPPORT.md / GOVERNANCE.md
├── CHANGELOG.md / ROADMAP.md / AUTHORS.md / NOTICE.md / CITATION.cff
└── LICENSE                       MIT（僅涵蓋原創部分）
```

## 2. 共同協議層（三個成品共用的大腦）

三個成品不共用程式碼，但共用同一套協議常數與流程。任何改動都應同時檢查這三處：

| 常數 / 行為 | 值 | 出現位置 |
|---|---|---|
| API base | `https://api.sjrc.club/` | `web/SJRC_AutoReader.html:293`、`pc/src/main.cpp:38`、APK 內的 `RetrofitClient` |
| `version` 標頭 | `2.9.0` | web `api()`、WinHTTP `httpCall()` |
| `x-current-platform` | `android` | 同上 |
| `x-current-locale` | `zh-HK` | 同上（官方用 `LanguageManage` 產生） |
| `x-auth-schema` | `CLIENT` 或 `PUBLIC` | 每個請求明確指定 |
| `authorization` | 僅 `CLIENT` + token 非空時 `Bearer <token>` | 同上 |
| 心跳間隔 | 3 秒（可調 1–600） | web `hbMs`、PC `hb`、官方 3000 ms |
| 心跳 `time` | **秒**，預設 3 | web `heartbeat()`、PC `workerLoop()` |
| 翻頁間隔 | 20 秒（可調 2–3600） | web `turnMs`、PC `turn` |
| 配額單位 | **秒**（28800 = 8 小時） | `fmtHours()`、`fmtHours()` 等價函式 |

### 2.1 請求管線（官方客戶端）

```text
UI / ViewModel
  └─ MainHttpService（Retrofit interface）
       └─ ParameterInterceptor        注入 version / locale / platform / Bearer
            └─ TokenExpireInterceptor  code 10117/10119 → refresh → 重送一次
                 └─ OkHttp           60 s timeout · followRedirects=false · 無 pinning
                      └─ https://api.sjrc.club/
                           └─ NetResponse { code, data, message, totalCount, isRefresh }
                                isSucc() == (code == 200 || code == 201)
```

重點：`authorization` **不是無條件注入**。`ParameterInterceptor` 只在
`x-auth-schema == "CLIENT"` 且 token 非空時加 Bearer；公開路由（`v1/public/*`）
必須送 `PUBLIC`，否則 NestJS guard 會回 `403 / 10104`。這也是為什麼測試公開端點
時要手動帶 `x-auth-schema: PUBLIC`。

### 2.2 閱讀 session 狀態機

```text
idle
  │ POST /v1/client/reading-time/start { bookId }
  ▼
running（sessionId 已取得）
  │ 每 3 s：PATCH /v1/client/reading-time { sessionId, time: 3 }
  │ 每 20 s：POST /v1/client/reading-pos { bookId, pos }
  │
  ├─ totalQuota < usedQuota        → 80008，停止
  ├─ (total - used) <= total * 20% → -777777，提示剩餘不足
  ├─ 3 次心跳失敗，第 4 次          → -1001，強制停止
  ├─ 達到目標時數                   → stopAll() → end
  └─ 使用者按停止 / 關閉             → end
  ▼
PATCH /v1/client/reading-time/end { sessionId }
  → idle
```

官方 `BookReadViewModel` 的 `startCountDown()` 使用
`setMillisInFuture(9999999000)` 與 `setCountDownInterval(3000)`，每個 tick 呼叫
`readTimeUsed()`；失敗時 `time = 3 * errorTime`，用來追回漏報的秒數。本專案沿用
「3 秒、time=3」的規則，但把「連續失敗次數」限制為 3 次後停止。

### 2.3 認證流程

```text
① Google（web / Windows helper）
   Google Identity Services → id_token
   POST v1/public/accounts/users/google/pre-login { providerId, type:"web" }
       ├─ data.accessToken → 完成
       └─ data.ticket      → 使用者輸入電郵驗證碼
            POST .../google/login { ticket, verificationCode }
            → accessToken + refreshToken

② 電郵 + 密碼（Android 外殼；網頁與 Windows 不提供）
   POST v1/public/accounts/users/email/pre-login { email, base64(password) }
       ├─ accessToken → 完成
       └─ ticket      → .../email/login { ticket, verificationCode }

③ refresh
   POST v1/public/accounts/users/token/refresh { refreshToken }
```

`ticket` 是一次性的。只按「發送驗證碼」不會產生 ticket；沒有 ticket 就不能兌換
驗證碼。這是實測 `Missing ticket` 的原因。

### 2.4 使用的端點（最小集合）

| 方法 | 路徑 | 用途 |
|---|---|---|
| `GET` | `/v1/client/reading-time` | 讀取 total/used 配額（秒） |
| `POST` | `/v1/client/reading-time/start` | `{bookId}` → `{sessionId}` |
| `PATCH` | `/v1/client/reading-time` | `{sessionId,time}`，time 為秒 |
| `PATCH` | `/v1/client/reading-time/end` | **`{sessionId}`**（不是 bookId） |
| `POST` | `/v1/client/reading-pos` | `{bookId,pos}`，每次翻頁 |
| `GET` | `/v2/client/books/{id}` | 書名、作者、章節 |
| `GET` | `/v1/client/bookshelf/books?page=1&limit=50` | 優先選書來源 |
| `GET` | `/v1/client/homepage` | 書架不足時的備援 |
| `GET` | `/v1/client/users/detail` | 帳號資料、登入驗證 |
| `POST` | `/v1/public/accounts/users/email/pre-login` | 電郵登入（Android） |
| `POST` | `/v1/public/accounts/users/email/login` | 驗證碼換 token（Android） |
| `POST` | `/v1/public/accounts/users/google/pre-login` | Google id_token 換 token/ticket |
| `POST` | `/v1/public/accounts/users/google/login` | 驗證碼換 token |
| `POST` | `/v1/public/accounts/users/token/refresh` | 續期 |

完整 98 個方法、67 個唯一端點見 [`data/api-endpoints.csv`](data/api-endpoints.csv)。
公開 Swagger：<https://api.sjrc.club/swagger-json>。

## 3. 網頁版：`web/SJRC_AutoReader.html`

這不是「先寫原始碼再打包」的專案——**這個 HTML 就是成品，也是唯一的原始碼**。
它分成四段：CSS 設計 token、登入卡、參數/書本/進度/日誌卡、以及最後的 `<script>`。

### 3.1 狀態物件

```js
const S = {
  token, refreshToken, ticket, user,
  running, stopRequested, sessionId, book, chapters,
  pagesPerChapter, totalPages, page, turnCount, bookCount,
  hbCount, errorTime, sentMs, refreshing,
  timer, lastHbAt, nextTurnAt, demo,
  quotaTotal, quotaUsed,
};
```

幾個命名容易誤導但單位是秒的欄位：`sentMs` 實際累加的是**毫秒**（給進度條用），
`serverUsedMin` / `baseUsedMin` 名字有 `Min`，但存的是**秒**。顯示一律經
`fmtHours(sec)` 換算。閱讀時不要把 `totalQuota` 當分鐘。

### 3.2 關鍵函式索引

| 行 | 函式 | 職責 |
|---:|---|---|
| 326 / 327 | `$` / `clamp` | DOM 與數值邊界 |
| 332 | `fmtHours` / `fmtHMS` | 秒 → 小時 / `Hh Mm Ss` |
| 343 | `log` | 日誌，最多保留 400 行 |
| 361 | `api` | fetch 包裝：標頭、schema、Bearer、AbortController、JSON 解析 |
| 398 | `isSucc` / `codeOf` / `msgOf` | 官方 `code==200||201` 判定 |
| 403 | `refreshAccessToken` | `POST token/refresh`，更新 localStorage |
| 457 | `initGsi` / `tryPromptOneTap` | Google GIS 初始化（需 http origin） |
| 551 | `onGoogleCredential` | id_token → pre-login |
| 591 | `preLogin` | Google pre-login；直接取得 token 或保存 ticket |
| 658 | `loadAccount` | `GET users/detail` |
| 683 | `loadReadingTime` | 讀配額，所有欄位依秒解釋 |
| 714 | `collectBooks` | 遞迴挖掘書架／首頁的書籍物件 |
| 781 | `startBook` | 取詳情 → 開 session → 初始位置 |
| 840 | `sendReadPos` | 以「估算分頁」組 `chapterId/chapterLocation/position` |
| 872 | `endBook` | 送 `sessionId`，清 localStorage session |
| 889 | `heartbeat` | 送 `{sessionId,time:secs}`；demo 模式只累加本機 |
| 942 | `onHeartbeatError` | 10117/10119 先 refresh；其餘累加錯誤，>3 停止 |
| 991 | `tick` / `tickInner` | 1 秒驅動器；防重入；提前 90 秒續期 |
| 1024 | `turnPage` | 翻頁、送位置、讀完換書 |
| 1043 | `nextBook` | 隨機選一本不同 id 的書 |
| 1065 | `startRun` | 驗證參數、開書、啟動 timer |
| 1112 | `stopAll` | 清 timer、送 end、更新 UI |

### 3.3 計時與併發

只有一個 `setInterval(..., 1000)` 驅動 `tick()`；`tickBusy` 防止上一個 tick 未完成
時重入。心跳與翻頁各自用 `lastHbAt` / `nextTurnAt` 與 `Date.now()` 比較，因此
瀏覽器節流（背景分頁）後不會一次補發大量請求。`api()` 每次建立新的
`AbortController`，預設 30 秒 timeout。

### 3.4 書本與位置模型

官方位置是「章節內頁碼 / 章節總頁數」。本專案沒有真實版面，因此用使用者設定的
`pagesPerChapter` 估算：

```js
chapterIndex = floor(page / pagesPerChapter)
pageInChapter = (page % pagesPerChapter) + 1
position = pageInChapter / pagesPerChapter
```

`chapterId` 送字串化的 `chapterIndex`，`chapterLocation` 送 `pageInChapter`，
`position` 保留 4 位小數。這和官方 `MagazineActivity.reportReadPos` 的欄位形狀
一致，但數值是近似值。

### 3.5 持久化

| Key | 內容 | 清除時機 |
|---|---|---|
| `sjrc.token` | accessToken | 登出 / 過期 |
| `sjrc.refresh` | refreshToken | 登出 / 過期 |
| `sjrc.ticket` | 一次性 ticket（sessionStorage） | 兌換後 / 關閉分頁 |
| `sjrc.session` | `{sessionId, bookId}` | end / 登出 / 開新書 |

Google Client ID 也存在 localStorage（key 由 UI 決定），它不是秘密，但也不應
提交到儲存庫。

### 3.6 已知限制

- `file://` 的 origin 是 `null`，Google GIS 不會工作；必須用 HTTP 伺服器。
- 沒有 bundler、沒有框架、沒有測試框架；務實但不利于大型重構。
- 進度條同時參考「本機累加」與「伺服器差值」，兩者可能短暫不同步。
- 背景分頁的 timer 會被瀏覽器節流；長時間執行建議保持前景或使用 Windows 版。

## 4. Android 外殼：`phone/`

### 4.1 APK 組成

`phone/SJRC-AutoRead.apk` 只有 8 個 entry：`AndroidManifest.xml`、`classes.dex`、
`resources.arsc`、`assets/index.html`、`res/mipmap/ic_launcher.png`、三個簽章檔。
它是一個**外殼**，不是完整閱讀器。

Manifest 關鍵值（以 `tools/axml.py` 解碼）：

| 欄位 | 值 |
|---|---|
| package | `club.sjrc.autoread` |
| versionCode / versionName | `1` / `1.0` |
| minSdk / targetSdk | `23` / `33` |
| permissions | `INTERNET`、`ACCESS_NETWORK_STATE` |
| application | `allowBackup=true`、`usesCleartextTraffic=true` |
| launcher activity | `club.sjrc.auto.MainActivity` |

> 注意：官方 SJRC 客戶端是 `club.sjrc.app`、`minSdk 26`、`targetSdk 35`。兩者
> 是不同的 APK，文件與 issue 中不要混用。

### 4.2 MainActivity 的核心

```smali
new-instance v0, Landroid/webkit/WebView;
invoke-virtual {v1, v2}, Landroid/webkit/WebSettings;->setAllowFileAccessFromFileURLs(Z)V
invoke-virtual {v1, v2}, Landroid/webkit/WebSettings;->setAllowUniversalAccessFromFileURLs(Z)V
const-string v1, "file:///android_asset/index.html"
invoke-virtual {v0, v1}, Landroid/webkit/WebView;->loadUrl(Ljava/lang/String;)V
```

`setAllowUniversalAccessFromFileURLs(true)` 讓 `file://` 頁面可以發 `fetch` 到
`https://api.sjrc.club`；沒有這個設定，整個 app 只會顯示空白。這也是一個安全
取捨：只載入受信任的本機 asset，不載入外部 URL。

`onStop()` 會呼叫 JS 的 `__appPause()`，避免退到背景後仍持續發包。

### 4.3 為什麼沒有 Google 登入

Google 封鎖嵌入式 WebView 的 OAuth（`disallowed_useragent`）。官方 App 使用原生
Google Sign-In，需要在 SJRC 的 Google Cloud 專案註冊 package name 與簽章 SHA-1；
本專案沒有那個權限。因此 Android 版只提供電郵 + 密碼 + 驗證碼。

### 4.4 原始碼完整性

`phone/src/` 保留 manifest、嵌入的 `index.html`，以及官方客戶端 6 個關鍵 smali
節錄（`BodyBuilder`、`BookReadViewModel`、`ParameterInterceptor`、
`RetrofitClient`、`TokenManager`、`UseTimeEntity`）。外殼 APK 內另有
`OAuthClient`、`WebServer`、`Probe`、`Poster`、`PopupChrome` 等輔助類別；其完整
反編譯原始碼未隨倉庫提供，但 APK 本身可被獨立檢查。

## 5. Windows 客戶端：`pc/src/main.cpp`

單一 C++17 檔案，約 2,300 行，沒有框架。分成五層：

| 區域 | 行（約） | 內容 |
|---|---:|---|
| 工具層 | 88–115 | UTF-8/UTF-16 轉換、`fmt`、時間字串 |
| JSON | 117–263 | 手寫 `JVal` / `JParser`，不依賴 nlohmann |
| HTTP | 265–337 | WinHTTP：8/8/10/15 秒 timeout、`followRedirects=false` |
| 業務 | 339–937 | 配額、選書、session、心跳、翻頁、目標停止 |
| UI | 939–2359 | Win32 控制項、GDI+ 繪製、OAuth helper server |

### 5.1 登入與 token

- 支援「貼上 token」：`tryImportText()` 會嘗試從 JSON 或純文字取出 token。
- 支援本機登入頁：`openOAuthPage()` 啟動 loopback HTTP server，預設
  `47821`；按 Google 登入時臨時監聽 `8080`（官方白名單只認這個 origin）。
- 登入完成或閒置（主頁 2 分鐘、Google 3 分鐘）即關閉 server。
- 設定與 token 存在 `HKCU\Software\SJRC\AutoReader`，值名 `token` 等。
- 啟動時會載入上次 token，並用 `GET /v1/client/users/detail` 驗證。

### 5.2 背景工作執行緒

`workerLoop(targetX10, hbSec, turnSec)`（`pc/src/main.cpp:806`）：

1. 驗證 token；10117/10119 先 `refreshToken()` 一次。
2. `loadUserDetail()` + `loadQuota(true)`。
3. `fetchAndStartRandomBook()` 選一本書並開 session。
4. 進入迴圈：每 150 秒主動 refresh；每 `hbSec` 送心跳；每 `turnSec` 翻頁；
   每次迴圈檢查 `sentSec >= targetX10 * 360`（targetX10 是 0.1 小時）。
5. 結束時 `endSession()`，無論正常停止或錯誤。

Shared state 由 `g_mtx` 保護，UI 透過 `setSnap()` + `requestRepaint()` 更新。
關閉視窗會設定 `g_stop` 並等待 worker 收尾。

### 5.3 靜態連結

`build.bat` 使用：

```bat
windres app.rc -O coff -o app.res.o
g++ -O2 -std=c++17 -municode main.cpp app.res.o -o SJRC_AutoReader.exe ^
    -static -static-libgcc -static-libstdc++ ^
    -lwinhttp -lcomctl32 -lws2_32 -lole32 -luser32 -lgdi32 -lshell32
```

`-static` 讓 EXE 不依賴 `libstdc++-6.dll` / `libgcc_s_seh-1.dll`；這是它能成為
「單檔、免安裝」的原因。`-municode` 對應 `wWinMain`。

## 6. 官方客戶端原始碼節錄：`phone/src/*.smali`

這些檔案是**協議證據**，不是本專案的執行碼。

| 檔案 | 關鍵內容 |
|---|---|
| `ParameterInterceptor.smali` | 何時注入 Bearer、locale、platform、version |
| `TokenManager.smali` | access/refresh token 儲存與 10 分鐘節流 |
| `RetrofitClient.smali` | base URL、60 秒 timeout、`followRedirects(false)`、無 pinning |
| `BodyBuilder.smali` | Map → Gson → `application/json;charset=UTF-8` |
| `BookReadViewModel.smali` | `startCountDown`、`readTimeUsed`、`readTimeEnd`、錯誤處理 |
| `UseTimeEntity.smali` | `totalQuota`/`usedQuota` 與 20% 判定 |

### 6.1 心跳的 smali 證據

```smali
iget v0, p0, ->errorTime:I
const-wide/16 v1, 3          ; 基礎值 3 秒
if-lez v0, +004h
int-to-long v3, v0
mul-long/2addr v1, v3        ; time = 3 * errorTime
```

### 6.2 `end` 的官方 bug

官方 `readTimeEnd()` 送 `{bookId}`，但 Swagger 的 `EndReadingDto` 必填 `sessionId`；
實測會回 `80009 reading session not exist`。本專案三個成品都改送 `sessionId`。
文件中保留這個差異，因為它是理解「為什麼不能照抄官方請求」的關鍵。

### 6.3 配額的 20% 判定

`UseTimeEntity.isRemainTimeLessThan20()` 比較的是
`remain <= total * 0.2`，不是「剩 20 分鐘」。函式名容易誤導，文件與程式都應以
比例為準。

## 7. 反編譯素材：`sjrc-source/`

- 來源：`SJRC_2.9.0.apks` 的 `base.apk`，6 個 DEX。
- 工具：androguard 4.1.4 + 自寫 AXML/ARSC/smali 匯出器（見 `BUILD.md`，未隨
  倉庫提供）。
- 規模：4,888 個 class，43 MB；套件分佈：`club.sjrc.app` 2,952、
  `com.reader.epic` 1,428、`com.skytree` 375、`com.middle` 95、
  `com.sjrc.baseapp` 28、`club.bookcentro.test` 10。
- 保真度：smali 是無損的；DAD 還原的 Java 不可直接編譯。
- 不含：原始 DEX、`resources.arsc` 二進位、`lib/*.so`、書籍內容；金鑰素材已替換為佔位符。

## 8. 資料與工具

| 路徑 | 用途 |
|---|---|
| `data/api-endpoints.csv` | 人可讀的端點表；適合 grep / Excel |
| `data/api-endpoints.json` | 機器可讀；含 class/method/http/path |
| `data/api-endpoints-by-module.md` | 按前綴分組的統計 |
| `data/source-stats.json` | 版本、SDK、smali 檔數、成品 SHA-256 |
| `SHA256SUMS` | 三個成品與關鍵資料的校驗碼 |
| `scripts/check_repo.py` | 連結、文件中英配對、資料一致性、校驗碼、SVG/XML、秘密掃描 |
| `scripts/package_release.py` | 可重現的 zip（固定時間戳、排序、排除快取） |
| `scripts/serve_web.py` | 帶正確 MIME 與 no-cache 的本機 HTTP 伺服器 |
| `Makefile` | `make check` / `make serve` / `make package` |

## 9. 錯誤碼與處理

| Code | 意義 | 本專案行為 |
|---|---|---|
| `10102` | DTO 驗證失敗 | 檢查 `time` 是否為秒、`sessionId` 是否空 |
| `10104` | `x-auth-schema` 不匹配 | 公開路由改 `PUBLIC` |
| `10117` / `10119` | token 無效 / 過期 | 先 refresh；失敗才停止 |
| `20002` | Google id_token 被拒 | 檢查 client type / origin |
| `20042` | refresh token 過期 | 只能重新登入 |
| `20043` | session not found | 清掉殘留 refreshToken |
| `20045` | 帳號未註冊 | 顯示訊息，不重試 |
| `80008` | 配額不足 | 停止 |
| `80009` | reading session 不存在 | `end` 改送 `sessionId` |
| `-777777` | 剩餘 ≤ 總量 20% | 提示 |
| `-1001` | 連續失敗 >3 | 強制停止 |
| `-1` | 本專案的網路錯誤 | 累加錯誤，>3 停止 |

## 10. 安全與隱私摘要

- Token 存放：web `localStorage`；Android WebView 同源 storage；Windows
  `HKCU\Software\SJRC\AutoReader`。**沒有** OS 級加密，這是已知取捨。
- 沒有後端、沒有遙測、沒有第三方分析。唯一的網路目的地是 `api.sjrc.club`
  （Google 登入另加 `accounts.google.com` / `www.gstatic.com`）。
- Windows `followRedirects=false`，避免 token 被轉送到其他主機。
- 儲存庫不含 token、密碼、私鑰、AES 金鑰、書籍內容。
- 文件描述 DRM 機制是為了互通性分析，不提供金鑰或解密步驟。

## 11. 建置與驗證

```bash
# 文件/資料/連結/校驗碼（不需要網路）
python3 scripts/check_repo.py

# 本機開網頁
python3 scripts/serve_web.py

# 抽出 script 做語法檢查（需要 Node）
python3 - <<'PY'
import re
s=open('web/SJRC_AutoReader.html').read()
open('.web-check.js','w').write(re.search(r'<script>(.*)</script>', s, re.S).group(1))
PY
node --check .web-check.js && rm .web-check.js

# 打包原始碼
python3 scripts/package_release.py
```

Windows 重建見 [`pc/src/README.md`](pc/src/README.md)；Android 重建需要 SDK 或
手動 smali 工具，見 [`docs/en/building.md`](docs/en/building.md)。

## 12. 已知落差與維護注意

1. `phone/src/` 沒有 `MainActivity.smali` 完整檔，只有 README 中的節錄；外殼
   APK 內有 6 個類別。若要做完整重建，需從 APK 反組譯。
2. `repo/sjrc-source/net_endpoints.json` 的 `hdr` 欄位是損壞的 androguard 物件
   字串；`data/api-endpoints.*` 已排除它，只保留可靠欄位。
3. 文件中「官方 minSdk 26 / 外殼 minSdk 23」必須分開寫，不要合併成一個數字。
4. 網頁版只依賴 Google GIS 登入；沒有網路或 origin 不在白名單時就無法登入
   （但模擬模式仍可操作 UI）。電郵登入只存在於 Android 內嵌頁。
5. 這是互通性研究，不是官方 App 的替代品；不要承諾「不會被偵測」或「不會被
   封鎖」。

## 13. 授權

原創程式碼與文件以 MIT 釋出（見 [`LICENSE`](LICENSE)）。反編譯素材、SJRC 名稱、
書籍內容與第三方程式碼**不在** MIT 範圍內，詳見 [`NOTICE.md`](NOTICE.md)。
