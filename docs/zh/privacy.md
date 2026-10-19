# 隱私

**繁體中文** · [English](../en/privacy.md)

## 簡短版

本專案沒有後端、沒有遙測、沒有分析。它在你的裝置本機處理 SJRC 帳號資料，只把
維持閱讀 session 所需的請求送到 `api.sjrc.club`。Google 登入另會連線 Google。

![資料流與信任邊界](../assets/data-flow.zh.svg)

## 資料清單

| 資料 | 來源 | 用途 | 存放 | 保留 |
|---|---|---|---|---|
| 電郵／密碼 | 你 | 登入 | 不儲存 | 僅請求期間 |
| Google ID token | Google Identity Services | 登入 | 記憶體 | 登入流程 |
| Access token | API | 認證請求 | localStorage／登錄檔 | 登出或覆寫前 |
| Refresh token | API | 續期 | localStorage／登錄檔 | 登出或覆寫前 |
| 驗證 ticket | API | 完成 2FA | sessionStorage／記憶體 | 一次兌換／關閉分頁 |
| 姓名、電郵、學校、身份 | API | 顯示帳號卡 | 記憶體／UI | Session |
| 總量／已用配額 | API | 進度 | 記憶體；session ID 持久化 | Session |
| 閱讀位置 | 本機估算，送到 API | 回報進度 | 記憶體 | Session |
| 書名／作者 | API | 顯示 | 記憶體 | Session |
| 日誌 | 本機產生 | 診斷 | DOM／文字框 | 清空／關閉前 |

應用不會把資料寫入檔案。除了下列 API 主機，不會送到其他地方。

## 網路目的地

| 目的地 | 時機 | 送出資料 |
|---|---|---|
| `https://api.sjrc.club/` | 所有操作 | 標頭、憑證、session ID、閱讀時間／位置、book ID |
| `https://accounts.google.com/` | Google 登入（網頁／登入頁） | Google OAuth 流程 |
| `https://www.gstatic.com/` | 載入 Google Identity Services | 一般網頁請求 metadata |

沒有廣告、分析、當機回報或更新檢查端點。

## Cookie 與瀏覽器儲存

網頁版不設 cookie。它用 `localStorage` 存四個 key，`sessionStorage` 存 ticket。
Google GIS script 可能依 Google 政策設自己的 cookie 或儲存；這不在本專案控制。

## Windows 登錄檔

設定與 token 以純字串存在 `HKCU\Software\SJRC\AutoReader`。能讀你的 Windows
profile 的人就能讀它們。移除：

```bat
reg delete "HKCU\Software\SJRC\AutoReader" /f
```

## 上游與第三方

SJRC（服務營運方）會收到請求，並可能依其政策記錄。使用 Google 登入時，Google
會收到 OAuth 流程。本專案與兩者都沒有協議，也無法刪除他們持有的資料。

## 你的控制

- 登出可從應用儲存移除 token。
- 清除 `localhost:8080` 的網站資料可移除網頁儲存。
- 刪除登錄鍵可移除 Windows 設定。
- 解除安裝 Android App 可移除 WebView 儲存。
- 不想讓 Google 參與就不要用 Google 登入。電郵密碼登入只存在於 Android 外殼；
  另外兩個成品沒有。

## 日誌與截圖

日誌可能含書名、session ID 與錯誤訊息。它不會刻意含 token。分享前：

- 遮蔽 `Bearer …`、`eyJ…`、ticket、session ID、電郵、學校名稱與書名。
- 裁掉帳號卡。
- 不要分享 token 欄位的截圖。

## 兒童與機構使用

本專案不是針對兒童。若在學校使用，學校政策與平台條款適用。維護者無法判定你的
使用是否被允許。

## 變更

若未來版本加入任何收集，必須在發佈前更新本頁。加入遙測卻不更新本頁的 pull
request 應被拒絕。
