# 快速開始

**繁體中文** · [English](../en/getting-started.md)

## 這是什麼

SJRC AutoReader 是一個小型、無第三方執行期依賴的客戶端，只處理 SJRC（金閱閣校園版）
API 中的閱讀時間部分。它重現官方 App 的心跳與翻頁，讓閱讀 session 持續累積
時間，直到你設定的目標。

它**不是**閱讀器：不下載、不解密、不儲存、不顯示書籍內容。沒有伺服器，也沒有
遙測。

> **執行之前**：自動累積閱讀時數可能違反學校規定或平台服務條款。不確定就先問。
> 見[法律](legal.md)。

## 需求

| 成品 | 需求 |
|---|---|
| 網頁 | 現代瀏覽器（Chrome、Edge、Firefox、Safari）。套件內啟動器需要 Python 3，才能啟動 Google 登入所需的本機 HTTP 伺服器。 |
| Android | Android 6.0+（API 23+），允許安裝未知來源。 |
| Windows | Windows 10/11 x64，免安裝、免執行期。 |

還需要 SJRC 帳號與可用的登入方式。Google 登入可用於網頁版與 Windows 本機登入頁；
Android 外殼只支援電郵密碼 + 驗證碼，因為 Google 封鎖嵌入式 WebView 的 OAuth。

## 五分鐘首次執行（網頁）

下載[網頁套件](https://github.com/infycat/SJRC-AutoReader/releases/latest/download/SJRC-AutoReader-Web.zip)並解壓縮。
Windows 雙擊 `Start-SJRC-AutoReader.bat`（需安裝 Python 3）；macOS/Linux 在解壓資料夾
執行 `python3 serve_web.py`。頁面會在 `http://localhost:8080/SJRC_AutoReader.html` 開啟。

1. 用 Google 登入。網頁版沒有電郵密碼表單。
2. 第一次執行把**目標時數**留在 `2`。
3. **心跳**保持 `3` 秒，**翻頁**保持 `20000` 毫秒。
4. 勾選**模擬模式**，觀察流程但不發包。
5. 取消模擬，按**開始**，看日誌與進度卡。

按**停止**結束。應用會先送 `PATCH /v1/client/reading-time/end`，再把 session
標記為關閉。

## 五分鐘首次執行（Android）

1. 下載並解壓[Android 套件](https://github.com/infycat/SJRC-AutoReader/releases/latest/download/SJRC-AutoReader-Android.zip)，再把 `SJRC-AutoRead.apk` 複製到裝置。
2. 在檔案管理員允許安裝未知來源。
3. 安裝、開啟，用電郵 + 密碼登入。
4. 設定目標後按開始。請保持前景；Android 可能節流背景 timer。

若畫面空白，代表建置缺少 WebView flag；已提交的 APK 有這些設定。見
[疑難排解](troubleshooting.md)。

## 五分鐘首次執行（Windows）

1. 下載並解壓[Windows 套件](https://github.com/infycat/SJRC-AutoReader/releases/latest/download/SJRC-AutoReader-Windows.zip)，再執行 `SJRC_AutoReader.exe`。
2. 用**本機登入頁**做 Google 登入，或貼上 access token。
3. 設定目標時數、心跳秒數、翻頁秒數。
4. 按**開始**。關閉視窗會停止 worker 並結束 session。

執行檔是靜態連結，不需要 Visual C++ 可轉散發套件。

## 第一次執行後要檢查什麼

- `GET /v1/client/reading-time` 的 `usedQuota` 增加量大致等於經過的秒數。
- 日誌每 3 秒（或你設定的間隔）出現一次心跳。
- 每次翻頁有一行 `POST /v1/client/reading-pos`。
- 停止時出現 `PATCH /v1/client/reading-time/end`，代碼 200/201。

若 `usedQuota` 一次跳 60 倍，或看到 `10102`，代表你送了毫秒而不是秒。讀
[API 參考](api.md#單位)。

## 接下來

- [安裝](installation.md) —— 校驗碼、卸載、平台注意事項
- [設定](configuration.md) —— 每個旋鈕與影響
- [使用](usage.md) —— 安全、可重複的執行流程
- [安全](security.md) —— 工具能與不能保護什麼
