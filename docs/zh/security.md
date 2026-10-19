# 安全

**繁體中文** · [English](../en/security.md)

## 範圍與非目標

這是一個連線遠端閱讀服務的本機客戶端。它**不是**安全邊界、密碼管理器或強化
瀏覽器。安全目標很窄：避免使用者的 token 意外洩漏、讓每個網路目的地可審核，
以及避免新增可能成為攻擊目標的後端。

## 資產

| 資產 | 敏感度 | 位置 |
|---|---|---|
| 密碼 | 高 | 只在登入表單；送到 API；本專案不儲存 |
| Google ID token | 高 | 登入期間在記憶體；送到 API |
| Access token | 高（bearer） | WebView/localStorage 或 `HKCU` |
| Refresh token | 高 | 同上 |
| 驗證 ticket | 中 | `sessionStorage` 或記憶體；一次性 |
| 帳號 profile | 中 | 顯示在 UI；本專案不持久化 |
| 閱讀時間／位置 | 低–中 | 送到 API；本機計數 |
| 日誌 | 視內容 | DOM／Windows 文字框；可能含書名 |

書籍內容與 DRM 金鑰**不是**本專案的資產，因為它從不處理它們。

## 對手

| 對手 | 能力 | 緩解 | 殘餘風險 |
|---|---|---|---|
| 其他本機使用者 | 讀瀏覽器 profile 或登錄檔 | 作業系統帳號隔離 | Token 以明文存放 |
| 裝置上的惡意程式 | 讀記憶體、儲存、鍵盤輸入 | 作業系統安全 | 完全入侵；範圍外 |
| 網路攻擊者 | 攔截或重新導向流量 | TLS 驗證；Windows 停用 redirect | 無憑證固定；受信任的 MITM 代理可讀 |
| 惡意依賴 | 在建置或執行期執行 | 客戶端無第三方執行期依賴；CI 審查 | 工具鏈入侵；範圍外 |
| 惡意貢獻 | 加入外傳程式碼 | 審查；`check_repo.py` 秘密掃描 | 堅定的貢獻者仍可藏邏輯 |
| 上游服務 | 看到請求、可能記錄 | 只送必要資料 | 上游記錄／保留不在我們控制 |
| 旁觀者 | 看螢幕 | 輸入後不再顯示 token | 日誌可能顯示書名 |

## 控制措施

### 網路

- 僅 HTTPS；base URL 寫死。
- Windows 客戶端設 `followRedirects=false`，30x 不能把 Bearer 標頭移到其他主機。
- 網頁客戶端用 `credentials: 'omit'`，不送 cookie。
- API host 是 `api.sjrc.club`。Google 登入另連 `accounts.google.com` 與
  `www.gstatic.com`。
- 沒有憑證固定。這避免憑證輪換時壞掉，代價是信任裝置的 CA 存放區。

### 憑證處理

- 密碼只用於一次 `pre-login` 請求，不寫入儲存。
- Access token 存在 `localStorage`（網頁／Android）或登錄檔（Windows），兩者
  在靜態時都是明文。
- 依設計，token 不會放進 URL 或日誌行。
- Refresh token 只送到 `token/refresh`。
- 登出會從儲存移除 token 與 refresh token。
- 關閉瀏覽器／Android App 不一定清除儲存；共用裝置請登出。

### 輸入與輸出

- 網頁日誌在插入伺服器文字前用 `escapeHtml()`。
- 書名與作者在 UI 用 `textContent`。
- 封面 URL 來自 API，會指定給 `img.src`；惡意 API 回應可能讓瀏覽器抓任意圖片
  URL。這是 metadata 隱私問題，不是 token 洩漏。
- Windows 客戶端用小型手寫 JSON parser。除了基本邊界檢查，並未針對對抗性輸入
  強化；不要把它指向不受信任的伺服器。

### 建置與發佈

- `SHA256SUMS` 綁定已提交成品。
- `scripts/check_repo.py` 拒絕文字檔中的類 JWT 字串、私鑰區塊與常見 token 模式。
- CI 每次 push 執行檢查器與網頁語法檢查。
- 發佈的二進位檔附在 tag，不會無聲替換。
- Android APK 以專案金鑰簽章。金鑰不在儲存庫。

## 刻意不保護什麼

- Token 靜態加密。
- 截圖／錄影保護。
- Root/jailbreak 偵測。
- 反除錯或混淆。
- 憑證固定。
- 伺服器端 attestation。

這些不是 bug，而是小型本機客戶端的範圍外項目。

## 安全使用檢查

- [ ] 執行下載的二進位檔前驗證 SHA-256。
- [ ] 共用裝置請用專用瀏覽器 profile 或 Windows 帳號。
- [ ] 不要把 token 貼進聊天、issue、截圖或 URL。
- [ ] 把裝置交給別人前登出並清除儲存。
- [ ] 不要在不受信任的 TLS 攔截代理網路上執行。
- [ ] 保持作業系統與瀏覽器更新。
- [ ] 任何「送 token 來驗證」的要求都是詐騙。本專案沒有需要 token 的客服。

## 回報漏洞

見 [SECURITY.zh.md](../../SECURITY.zh.md)。不要開公開 issue 附 token、payload
或 exploit。請遮蔽為 `eyJ…<redacted>`。

## 威脅模型變更

若未來版本加入後端、遙測、新主機或憑證分享，本頁必須在同一個 pull request
更新。沒有記錄理由就弱化模型，應被拒絕。
