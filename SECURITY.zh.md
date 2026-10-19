# 安全政策

**繁體中文** · [English](SECURITY.md)

## 支援版本

本專案提供三個小型成品，沒有長期支援分支。

| 版本 | 支援 |
|---|---|
| 1.0.x（目前） | ✅ |
| < 1.0 | ❌ |

## 回報漏洞

**不要開公開 issue。** 請使用 GitHub 的私下漏洞回報：

1. 進入儲存庫的 **Security** 分頁。
2. 選擇 **Advisories → Report a vulnerability**。
3. 說明受影響的成品（網頁／Android／Windows）、版本或 commit、重現步驟，以及
   你認為的影響。

若無法使用私下回報，請只開一個最小化的公開 issue，寫明「我需要私下安全聯絡」，
不要放任何細節、payload 或 token。

我們會在 **7 天**內確認收到，並在 **30 天**內提供狀態更新。這是目標，不是合約；
本專案由志願者維護。

## 範圍內

- 成品將憑證或 token 傳送到 `api.sjrc.club`（或網頁登入設定的 Google 身分端點）
  以外的主機
- Token、refresh token 或驗證碼被寫入日誌、截圖、錯誤訊息、URL 或提交的檔案
- 可能導致使用者洩漏憑證的文件敘述
- 執行不受信任輸入或發佈不一致二進位檔的建置腳本
- 本儲存庫自有腳本與 CI 的供應鏈問題
- 能讓應用程式執行文件所述閱讀時間與閱讀位置範圍以外動作的方法

## 範圍外

- 上游 SJRC 服務或官方 App 的漏洞。請向服務營運方回報。
- 速率限制、帳號封禁或服務條款執行。
- Token 以未經作業系統加密的方式存放於 `localStorage` / `HKCU`。這是已記錄的
  設計取捨，不是漏洞；威脅模型見 [docs/zh/security.md](docs/zh/security.md)。
- 對你自己機器或帳號的阻斷服務。
- 本機 `python3 -m http.server` 缺少安全標頭。
- 需要已遭入侵的裝置或系統管理員權限的問題。

## 安全港

對於符合以下條件的研究者，我們不會採取法律行動：

- 善意避免侵犯隱私、破壞資料與中斷服務
- 只對**自己的**帳號與裝置測試
- 在公開披露前給予合理修復時間
- 不存取、下載或再散布其他使用者的資料或書籍內容

## 我們絕不會索取

我們絕不會要求你提供真實的 access token、refresh token、密碼、驗證碼、私鑰或
已解密的書籍。請將 token 遮蔽為 `eyJ…<redacted>`，帳號識別碼遮蔽為
`<account-id>`。

## 安全設計要點

- 沒有後端、沒有遙測、沒有第三方分析。
- 網頁版直接連線 `https://api.sjrc.club/`。
- Android 外殼載入本機 asset，並發出相同的直接請求。
- Windows 版使用 WinHTTP，`followRedirects=false`。
- `scripts/check_repo.py` 會用 `SHA256SUMS` 與 `data/source-stats.json` 核對
  發佈的二進位檔。

完整威脅模型見 [docs/zh/security.md](docs/zh/security.md)。
