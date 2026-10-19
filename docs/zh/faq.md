# 常見問題

[English](../en/faq.md) · 繁體中文

先給短答案，再連到長解釋。若答案取決於你的帳號、學校政策或平台版本，會明確標出。

## 專案與範圍

### 這跟金閱閣（SJRC）有關係嗎？

沒有。這是一個獨立的互通性研究專案，未獲 SJRC、採用該平台的學校或出版社的
背書、支援或授權。名稱只用來描述軟體與誰通訊。

### 自動閱讀被允許嗎？

取決於平台服務條款與學校規定。很多情況下並不允許，帳號可能被限制或停權。
本專案以個人學習與協議研究為目的發表。執行前請讀
[法律與可接受使用](legal.md)，不確定就問學校。

### 會下載或解密書籍嗎？

不會。三個客戶端只送出閱讀時間與頁面位置，不下載書籍檔案、不請求 DRM 金鑰、
也不儲存解密內容。反編譯源碼顯示官方客戶端有內容加密層，但本專案既未實作，
也不附帶任何金鑰素材。見
[逆向分析](reverse-engineering.md#內容加密僅描述金鑰已移除)。

### 為什麼儲存庫裡有反編譯的客戶端程式碼？

官方客戶端以 Android APK 公開散布。反編譯 smali 用來還原標頭名稱、單位、
端點形狀與心跳狀態機。保留節錄讓這些主張可被稽核，也讓其他研究者能重現。
它只是參考素材，沒有任何建置會連結它。著作權仍屬原權利人；見
[NOTICE.md](../../NOTICE.md)。

### 有後端、代理或遙測服務嗎？

沒有。每個成品都直接連 `https://api.sjrc.club/`。本專案不營運伺服器、不內嵌
分析 SDK、沒有當機回報、也不收集帳號。Token 只從你的裝置送到 API，不會到別處。

### 該用哪個成品？

想要最容易稽核、用現代瀏覽器，選網頁版。需要手機且能用電郵密碼登入，選 Android。
想要原生執行檔與登錄檔設定，選 Windows。行為契約相同，差別只在宿主外殼。

## 帳號與登入

### 為什麼 Android 沒有 Google 登入？

Google 禁止在嵌入式 WebView 內使用 OAuth（`disallowed_useragent`）。官方 App
用原生 Google Sign-In，需要把套件名稱與簽章憑證註冊到 SJRC 的 Google Cloud
專案；本專案沒有那個權限。Android 請用電郵加密碼，或改用網頁版。

### 驗證碼收到了，為什麼還是登入失敗？

驗證碼不是獨立登入。順序是：電郵密碼或 Google 憑證 → `pre-login` 回傳
`ticket` → 輸入驗證碼 → `email/login` 或 `google/login` 用
`ticket + code` 換 token。單獨呼叫 `send-verifications` 只回
`{status:true}`，沒有 ticket，所以那種驗證碼無法兌換。見
[API 參考](api.md#認證)。

### 我的帳號沒有密碼，還能用嗎？

網頁版可以用 Google。Android 的電郵密碼登入需要密碼；Windows 則用本機 Google
輔助頁或貼上 token。請在官方
App（**我的 → 密碼 → 修改**）或平台帳號復原流程設定／重設。不要把你的密碼
交給任何人，包括 issue 裡。

### Token 除了 API 還會送到別的地方嗎？

不會。網頁版存在 `localStorage`；Android 外殼在 WebView 裡跑同一頁；Windows
存在 `HKCU\Software\SJRC\AutoReader`。沒有同步、遙測或遠端日誌。見
[隱私](privacy.md)。

### 可以在兩台裝置同時用同一個帳號嗎？

平台可能只允許一個活躍 session，或使較早的 token 失效。兩個客戶端同時對同一
帳號執行未經測試，也可能讓額度計算混亂。一次只用一個客戶端，先停掉第一個再
啟動第二個。

## 執行

### 閱讀時間是怎麼累積的？

開啟閱讀 session，每三秒送一次心跳
（`PATCH /v1/client/reading-time`，`time: 3`，單位秒），翻頁時回報位置，結束時
關閉 session。額度由伺服器擁有，客戶端不能自己造秒數。見
[使用](usage.md#讀懂狀態)。

### 為什麼心跳是 3 秒、翻頁是 20 秒？

3 秒是官方 2.9.0 客戶端的值（`setCountDownInterval(3000)`），送出的 `time` 也
一樣。20 秒是本專案預設的翻頁間隔，可調整，因為合適節奏取決於書長與額度。
更快不一定更好：請求可能失敗，伺服器也可能對可疑流量有不同的處理。

### 為什麼額度沒有動？

依序檢查：模擬模式已關；start 成功且有 `sessionId`；心跳確實送出（日誌有
`PATCH` 且 code 為 `200`）；你看的是正確帳號與額度欄位；帳號還沒用完
（`80008`）。沒關舊 session 就重開瀏覽器也可能留下逾時 session。見
[疑難排解](troubleshooting.md#配額不動)。

### 達到目標會怎樣？

客戶端完成當次心跳／翻頁，送 `PATCH /v1/client/reading-time/end`，清除活躍
session，停止計時器。不會自動再開一輪。若平台額度用完，不論目標是多少都會以
`80008` 停止。

### 目標單位是小時還是秒？

介面輸入**小時**，API 使用**秒**。`2.0` 小時會變成 `targetSec = 7200`，額度
欄位也以秒比較。`28800` 是 8.0 小時，不是 8 分鐘。見
[單位](api.md#單位)。

### 為什麼連續四次錯誤後就停了？

連續四次心跳失敗會觸發強制停止路徑（官方客戶端事件模型中的 `-1001`）。常見
原因是 token 過期、session 被刪、或網路中斷。重新登入換 token，再開新一輪；
不要一直重試同一個死掉的 session。

## 成品

### Android 需要哪個版本？

外殼 APK 宣告 `minSdk 23`（Android 6.0）、`targetSdk 33`。官方 SJRC 客戶端是
另一個 App（`club.sjrc.app`，`minSdk 26`）。外殼套件是
`club.sjrc.autoread`，可與官方 App 並存，但簽章不能與先前安裝過的外殼衝突。

### Windows 為什麼會跳警告？

它是未簽章、靜態連結的原生執行檔。SmartScreen 與部分防毒對任何未簽章執行檔都
會起疑。請對照 [`SHA256SUMS`](../../SHA256SUMS) 驗證 SHA-256，必要時自行掃描，
再選「仍要執行」。若組織禁止未簽章執行檔，請用網頁版。

### 需要先裝 Node、Python 或執行環境嗎？

網頁版套件的啟動器需要 Python 3，因為 Google 登入要求本機 HTTP origin；Windows
可雙擊 `Start-SJRC-AutoReader.bat` 啟動。Android APK 與 Windows 執行檔不需要額外
執行環境。Node 只在 CI 的選擇性語法檢查中使用。

### 為什麼雙擊網頁是空白的？

那個檔案設計成用 HTTP 提供。`file://` 的 origin 是 `null`，會讓 Google
Identity Services 失敗，也可能讓 `fetch` 失敗。下載並解壓網頁套件後，在 Windows
雙擊 `Start-SJRC-AutoReader.bat`；macOS/Linux 執行 `python3 serve_web.py`，再開
`http://localhost:8080/SJRC_AutoReader.html`。

### 沒有 Android SDK 也能重建 APK 嗎？

文件提供兩條路：用 `apktool` 加 `apksigner` 的修補流程，以及最小 Gradle 外殼
流程。已簽章的 APK 可直接執行，不必重建。見
[建置](building.md#android-外殼)。

## 疑難排解與貢獻

### 回報 bug 要附什麼？

成品與版本、作業系統、完整錯誤碼／訊息、去掉 token 與個資的相關日誌、最小重現
步驟。不要貼 token、密碼、驗證碼、電郵、校名或書名。使用 issue 表單並遵循
[SUPPORT.md](../../SUPPORT.md)。

### 安全性問題怎麼回報？

遵循 [SECURITY.md](../../SECURITY.md)。不要在公開 issue 放利用細節、token 或
個資。若問題屬於第三方服務，也請通知該服務；本專案修不了他們的伺服器。

### 可以貢獻翻譯或修正嗎？

可以。先讀 [CONTRIBUTING.md](../../CONTRIBUTING.md)。文件改動應讓英文與中文頁
結構對齊；`make check` 會檢查連結、對齊、資料檔與意外外洩的秘密。協議相關
改動應引用確切的 smali 類別或公開 Swagger 操作。

### 為什麼不加後端讓登入變簡單？

後端會收集帳號憑證或 token、製造單點故障，並徹底改變隱私模型。本專案刻意沒有
伺服器。若未來功能真的需要，必須以明確、可選擇的設計與威脅模型提出，不能偷偷
塞進客戶端。

### 要怎麼驗證一個發行版？

把壓縮檔與二進位檔的雜湊跟 `SHA256SUMS` 比對，檢視每個二進位旁附的源碼，並讀
發行說明與 `CHANGELOG.md`。需要高保證時，依 [建置](building.md) 從源碼重建，
比較行為而非位元組：不同編譯器會產生不同的執行檔。
