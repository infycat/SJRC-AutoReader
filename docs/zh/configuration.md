# 設定

**繁體中文** · [English](../en/configuration.md)

本專案沒有設定檔。每個成品都在 UI 提供設定，並存在本機。本頁列出每個旋鈕、
範圍、預設值與背後的儲存鍵。

## 網頁與 Android 欄位

| UI 欄位 | 內部 id | 預設 | 範圍 | 單位 | 作用 |
|---|---|---:|---|---|---|
| 目標時數 | `targetHours` | `2` | ≥ 0.1，步進 0.1 | 小時 | 累積到此值就停止 |
| 每章頁數 | `pagesPerChapter` | `20` | 1–200 | 頁 | 估算 `reading-pos` 用 |
| 心跳 | `hbMs` | `3` | 1–600 | **秒** | 兩次 `PATCH /reading-time` 的間隔；同值作為 `time` 送出 |
| 翻頁 | `turnMs` | `20000` | 2,000–3,600,000 | **毫秒** | 兩次翻頁與 `POST /reading-pos` 的間隔 |
| 模擬模式 | `demoMode` | 關 | 開/關 | — | 只跑 UI 與計數，不發包 |
| Google Client ID | `clientId` | 預填的公開 Web client | 字串 | — | 僅網頁；需與 Google Cloud 登記的 origin 一致 |
| 登入方式 | `loginType` | `web` | `web` / `android` / `ios` | — | 告訴後端 id_token 來自哪種 Google client；瀏覽器請用 `web` |

標籤寫心跳是秒、翻頁是毫秒；程式讀 `hbMs` 後乘 1000 用於本機計時，但送給 API
的 `time` 是原始秒數。不要「修正」成送 `hbMs * 1000`。

### 建議預設

| 情境 | 目標 | 心跳 | 翻頁 | 說明 |
|---|---:|---:|---:|---|
| 第一次模擬 | 0.1 h | 3 秒 | 20 秒 | 勾選模擬，不發包 |
| 一般 | 2 h | 3 秒 | 20 秒 | 對齊官方 3 秒節奏 |
| 保守 | 1 h | 3 秒 | 30 秒 | 較少位置回報 |
| 短測試 | 0.1 h | 3 秒 | 5 秒 | 看兩三次翻頁就停 |

## Windows 欄位

| 欄位 | 登錄值 | 預設 | 單位 | 作用 |
|---|---|---:|---|---|
| 目標時長 | `target` | `2` | 小時（0.1 精度） | 目標停止 |
| 心跳 | `hb` | `3` | 秒 | 心跳間隔與 `time` |
| 翻頁 | `turn` | `20` | 秒 | 翻頁間隔 |
| Token | `token` | 空 | 字串 | Access token |
| Refresh token | `refresh` | 空 | 字串 | Refresh token |

全部位於 `HKCU\Software\SJRC\AutoReader`，型別 `REG_SZ`。開始執行或導入 token
時由 UI 寫入。

## 寫死的行為

以下刻意不開放設定：

| 常數 | 值 | 原因 |
|---|---|---|
| API base | `https://api.sjrc.club/` | 唯一支援的服務 |
| 客戶端版本 | `2.9.0` | 必須符合 API 預期的版本 |
| Locale 標頭 | `zh-HK` | 與擷取的客戶端一致 |
| Platform 標頭 | `android` | 與擷取的客戶端一致 |
| API timeout（網頁） | 30,000 ms | 避免請求懸置 |
| API timeout（Windows） | 連線 8 秒、接收 15 秒 | WinHTTP 分階段 |
| Token 提前續期 | JWT 到期前 90 秒 | 避免心跳打到 401 |
| Windows 續期節奏 | 每 150 秒 | 保持伺服器 session |
| 連續錯誤上限 | 3 次（第 4 次停止） | 對齊官方強制登出 |
| 配額警告 | 剩餘 ≤ 總量 20% | 官方比例，不是 20 分鐘 |
| 日誌保留 | 400 行（網頁） | 控制 DOM 大小 |
| UI tick | 1 秒 | 單一排程器 |

## 儲存

### 網頁／Android

| Key | 儲存 | 內容 | 清除時機 |
|---|---|---|---|
| `sjrc.token` | `localStorage` | access token | 登出、手動清除網站資料 |
| `sjrc.refresh` | `localStorage` | refresh token | 登出、手動清除網站資料 |
| `sjrc.ticket` | `sessionStorage` | 一次性 ticket | 兌換驗證碼、關閉分頁 |
| `sjrc.session` | `localStorage` | `{sessionId, bookId}` | end、登出、開頁清理 |
| Google Client ID | `localStorage` | 公開 OAuth client id | 手動清除 |

Access token 是 bearer 憑證，能讀取瀏覽器 profile 的人就能拿到。若在意，請使用
專用瀏覽器 profile。

### Windows

全部在 `HKCU\Software\SJRC\AutoReader`，token 是純 `REG_SZ`。移除：

```bat
reg delete "HKCU\Software\SJRC\AutoReader" /f
```

## 重設

| 目標 | 網頁 | Android | Windows |
|---|---|---|---|
| 登出 | 登出按鈕 | 登出按鈕 | 清空 token 欄／刪登錄 |
| 忘記設定 | 清除網站資料 | 應用程式資訊 → 清除儲存 | 刪除登錄鍵 |
| 全部移除 | 刪檔 + 清除網站資料 | 解除安裝 | 刪 EXE + 登錄鍵 |

## 更改 API base

不要。本專案是為 `api.sjrc.club` 與特定客戶端版本而寫。把它指向其他服務的分支
是另一個專案，應同步更新文件、安全說明與測試。

## 關於 `pagesPerChapter`

它不影響閱讀時間，只影響 `reading-pos` 的內容：章節索引、章節內頁碼、`position`
比例。數值太大，看起來像在很長的章節裡翻頁；太小則快速換章。預設 `20` 是合理
估算，不是實際書本頁數。
