# 測試

**繁體中文** · [English](../en/testing.md)

## CI 執行什麼

```bash
make check
```

`scripts/check_repo.py` 執行離線檢查：

| 檢查 | 抓什麼 |
|---|---|
| 必要檔案 | 缺少 README、授權、文件、資料 |
| 相對連結 | `docs/en` ↔ `docs/zh` 壞連結、缺少圖片 |
| 文件中英配對 | 一頁只有一種語言，或標題數量不同 |
| JSON 合法性 | `data/*.json` 壞掉、端點欄位 |
| CSV 形狀 | 端點列有預期欄位 |
| SHA-256 | 已提交成品與 `SHA256SUMS`、`data/source-stats.json` 相符 |
| SVG/XML | 圖表格式錯誤 |
| 意外機密 | `eyJ…` 類 JWT、`-----BEGIN … PRIVATE KEY-----`、明顯 token |
| 禁止路徑 | `.git`、快取、編輯器備份 |

CI 也會對網頁 inline script 執行 `node --check`，並驗證 workflow YAML。它**不會**
呼叫真實 API。

## 本機檢查

```bash
python3 scripts/check_repo.py
python3 -m compileall -q scripts
```

檢查器失敗時會以非零退出，並印出確切路徑／行號。

## 網頁語法檢查

```bash
python3 - <<'PY'
import re
s = open('web/SJRC_AutoReader.html', encoding='utf-8').read()
open('.web-check.js','w',encoding='utf-8').write(re.search(r'<script>(.*)</script>', s, re.S).group(1))
PY
node --check .web-check.js && rm .web-check.js
```

## 單檔應用的靜態檢查

沒有測試框架。審查時保持以下不變量：

- `fetch(` 只打 `BASE`（搜尋 `fetch(`）。
- 沒有 token 被插進日誌或 URL。
- `localStorage.setItem` 只寫四個已記錄的 key。
- `innerHTML` 只用於靜態模板，不用於伺服器文字。日誌用 `escapeHtml()`。
- 每個請求明確設定 `x-auth-schema`。
- 心跳 body 的 `time` 是秒。
- `end` 送 `sessionId`。

## 手動測試矩陣

發佈前執行，記錄平台與建置雜湊。

| # | 情境 | 預期 |
|---:|---|---|
| 1 | 網頁用 HTTP，Google 登入 | 帳號卡填滿 |
| 2 | 網頁用 `file://` 開啟 | Google 按鈕顯示清楚的 origin 錯誤；模擬模式仍可用 |
| 3 | 已登記 origin 的 Google 登入 | 取得憑證，出現 token |
| 4 | 未登記 origin 的 Google 登入 | 顯示錯誤；不發請求 |
| 5 | 模擬模式，目標 0.1 h | 計數前進，日誌說模擬，無 HTTP |
| 6 | 真實執行，目標 0.1 h | 至少一次 start、數次心跳、至少一次位置、一次 end |
| 7 | 手動停止 | 送出 end、timer 清除、按鈕復原 |
| 8 | 達到目標 | 自動停止並 end |
| 9 | 無效 token | 10117 路徑、嘗試續期、清楚停止 |
| 10 | 執行中斷網 | 請求錯誤、錯誤計數、4 次後停止 |
| 11 | 故意把 `time` 送成毫秒（測試版） | 記錄 10102 與單位提示 |
| 12 | end 送 `bookId`（測試版） | 記錄 80009；送 `sessionId` 的版本成功 |
| 13 | Android 覆蓋安裝官方 App | 必須失敗；套件不同 |
| 14 | Android 空白頁 | 檢查 WebView flag；已提交 APK 可載入 |
| 15 | Android 背景 5 分鐘 | timer 可能被節流；不崩潰 |
| 16 | Windows SmartScreen | 顯示警告；執行前驗證雜湊 |
| 17 | Windows 登入頁遇 8080 被佔用 | 清楚錯誤；可手動導入 token |
| 18 | Windows 執行中關窗 | Worker 停止並送 end |
| 19 | 長時間執行（1 小時+） | 記憶體穩定；網頁日誌有上限 |
| 20 | 到期前續期 | 不計入心跳失敗 |

## Mock server 測試（建議下一步）

本專案沒有真實帳號測試。本機 mock 是正確的下一步投資：

```text
POST /v1/client/reading-time/start     -> {code:200,data:{sessionId:"s1"}}
PATCH /v1/client/reading-time           -> {code:200,data:{totalQuota:28800,usedQuota:3}}
POST /v1/client/reading-pos             -> {code:200}
PATCH /v1/client/reading-time/end       -> {code:200}
```

可斷言：

- 心跳 body 是 `time: 3`，不是 `3000`。
- end body 是 `sessionId`，不是 `bookId`。
- 10117 只觸發一次續期，然後重試。
- 連續四次失敗會停止迴圈。
- 目標 0 在任何請求前就被拒絕。

不要讓自動測試用真實帳號打真實服務。

## 發佈煙霧測試

1. 在乾淨 checkout 驗證 `SHA256SUMS`。
2. 執行檢查器。
3. 用全新瀏覽器 profile 開網頁，跑 0.1 h 模擬。
4. 在乾淨 Android profile 安裝 APK；驗證 package 與簽章；跑模擬。
5. 在乾淨 Windows VM 跑 EXE；確認無 DLL 依賴；跑模擬。
6. 確認 token 沒出現在 `git status`、截圖或日誌。

## 沒有測試什麼

- 上游 API 的未來行為
- Google Cloud 設定
- 各 Android OEM 的 WebView 差異
- 沒有網路的 Windows
- 非 x64 Windows
