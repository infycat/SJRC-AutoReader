# 貢獻指南

**繁體中文** · [English](CONTRIBUTING.md)

感謝你願意幫忙。本儲存庫刻意保持精簡：一個單檔網頁應用、一個極小的 Android
WebView 外殼、一個原生 Windows 客戶端，以及用來記錄協議的反編譯客戶端節錄。
這個形狀是特性，不是意外。

請先閱讀 [CODE_OF_CONDUCT.zh.md](CODE_OF_CONDUCT.zh.md)、
[SECURITY.zh.md](SECURITY.zh.md) 與[範圍限制](#範圍限制)，再開 pull request。

## 可以貢獻什麼

- 用 [錯誤回報表單](.github/ISSUE_TEMPLATE/bug_report.yml) 回報可重現的錯誤
- 改善文件、翻譯、圖表或範例
- 新增能抓住真實失敗的回歸測試或檢查器
- 重現平台問題，並附上**不含 token 或個人資料**的日誌
- 審查 pull request，特別是安全敏感的變更

歡迎功能建議，但本專案不會加入下載、解密、儲存或再散布書籍內容的功能，也不會
加入繞過認證或配額的功能。

## 範圍限制

本專案必須遵守以下界線：

1. **不取得內容。** 只呼叫閱讀時間與閱讀位置 API，不下載或解密書籍，也不附帶
   DRM 金鑰。
2. **不收集憑證。** 沒有伺服器、分析端點或遙測。Token 只留在使用者的瀏覽器
   儲存、Android WebView 儲存或 Windows 登錄檔中。
3. **不新增執行期依賴**，除非有書面提案。網頁版是 vanilla JS；Windows 版是靜態
   連結的 C++；Android 外殼極小。
4. **不編造 API 事實。** 協議變更必須有公開 Swagger、去敏後的實際回應，或反編譯
   客戶端作為依據，並說明是哪一種。
5. **不洗授權。** 反編譯 smali 仍屬原權利人所有，不得改以其他授權釋出，也不得
   複製到不同授權的專案。

## 開發環境

網頁與文件不需要安裝任何東西：

```bash
git clone <你的 fork 網址> SJRC-AutoReader-Repo
cd SJRC-AutoReader-Repo
python3 scripts/serve_web.py          # http://127.0.0.1:8080
python3 scripts/check_repo.py         # 離線文件與資料檢查
```

選用工具：

| 工具 | 用途 |
|---|---|
| Python 3.10+ | 檢查器、資料產生、打包 |
| Node.js 18+ | 對抽出的網頁 script 執行 `node --check`（僅 CI） |
| `g++`（MinGW-w64）與 `windres` | 重建 Windows 執行檔 |
| `apksigner`／Android SDK | 選用；已提交的 APK 已簽章 |

閱讀或審查 Android 外殼不需要 Android SDK。

## 儲存庫慣例

- 網頁版**成品即原始碼**：直接編輯 `web/SJRC_AutoReader.html`，不要引入 bundler。
- 各成品旁的 `web/src/README.md`、`phone/src/README.md`、`pc/src/README.md`
  說明佈局。
- 本儲存庫不散布完整反編譯參考樹。協議整理請維護在 `data/` 與雙語文件中。
- 文件是雙語。每個 `docs/en/X.md` 都需要對應的 `docs/zh/X.md`，且標題與連結
  集合一致。`scripts/check_repo.py` 會檢查。
- 程式識別字與 API 欄位名保持英文；說明文字可用英文或繁體中文。
- 日期使用 ISO 8601（`2026-10-19`）。需要時間時附上時區。
- 不要只有 emoji 的 commit，不要提交產生的 lockfile 或 `node_modules`。

## 進行變更

1. 比錯字或一行修正更大的變更，請先開 issue，說明問題、平台與已嘗試的方法。
2. Fork 並建立主題分支：`git checkout -b fix/heartbeat-timeout`。
3. 用最小變更解決問題，不要混入無關的格式調整。
4. 更新受影響文件的**兩個語言版本**，並在 `CHANGELOG.md` 的
   `## [Unreleased]` 下加一條。
5. 執行檢查：

   ```bash
   make check
   ```

6. 使用模板開 pull request，說明使用者可見的影響、測試方式與剩餘風險。

## Commit 訊息

使用簡短的祈使句主旨，可加上慣例前綴：

```text
fix(web): stop timer when the target is reached
docs(api): clarify that reading-time values are seconds
test(repo): reject docs/en pages without a zh counterpart
```

主旨不超過 72 個字元。變更不明顯時，在正文說明**為什麼**。不要在 commit 訊息或
截圖中包含 token、帳號識別碼或個人資料。

## 審查與合併

- CI 必須通過（檢查、網頁語法、文件中英配對、連結檢查、資料驗證）。
- 需要一位維護者批准。安全敏感變更可能需要第二位審查或私下討論。
- 預設由維護者 squash merge，讓主線歷史保持可讀。
- 合併不等於發佈。發佈流程見 [docs/zh/release.md](docs/zh/release.md)。

## 回報安全問題

漏洞、洩漏的 token，或可能影響其他使用者的繞過方式，**不要**開公開 issue。請依
[SECURITY.zh.md](SECURITY.zh.md) 處理。

## 授權

提交貢獻即表示你同意你的原創貢獻以 [LICENSE](LICENSE) 的 MIT 授權釋出。你保留
貢獻的著作權。第三方程式碼與反編譯素材**不會**被本專案重新授權；見
[NOTICE.md](NOTICE.md)。
