# 發佈流程

**繁體中文** · [English](../en/release.md)

## 版本

本專案對自己的程式碼使用語意化版本：

- **MAJOR** —— 使用者必須採取行動的變更（儲存格式、登入流程、套件名）
- **MINOR** —— 保持相容的新行為
- **PATCH** —— 文件、測試、內部修正

上游 SJRC 版本（`2.9.0`）是協議目標，不是本專案版本。兩個數字都會出現在文件，
不要混淆。

## 發佈前檢查

- [ ] 在乾淨 checkout 執行 `make check` 通過。
- [ ] `CHANGELOG.md` 在新版本下有條目與日期。
- [ ] `README.en.md` 與 `README.zh.md` 的功能與連結一致。
- [ ] `docs/en` 與 `docs/zh` 頁面集與標題一致。
- [ ] `SHA256SUMS` 與提交的二進位檔相符。
- [ ] `data/source-stats.json` 與大小、雜湊相符。
- [ ] `git status` 或截圖中沒有 token、私鑰或個人資料。
- [ ] 三個成品已在目標平台做煙霧測試。
- [ ] 若引用版本，更新 `CITATION.cff` 的版本／日期。
- [ ] 沒有 `[Unreleased]` 內容被無聲丟棄。

## 建立壓縮檔

```bash
python3 scripts/package_release.py --version 1.0.1
python3 scripts/package_downloads.py
```

腳本建立來源壓縮檔與三個平台套件：

- `SJRC-AutoReader-Repo-1.0.1.zip`：完整儲存庫快照。
- `SJRC-AutoReader-Web.zip`：HTML、本機 HTTP 伺服器、Windows `.bat` 啟動器、說明、授權與內部校驗碼。
- `SJRC-AutoReader-Android.zip`：APK、安裝說明、授權與內部校驗碼。
- `SJRC-AutoReader-Windows.zip`：EXE、使用說明、授權與內部校驗碼。
- 每個 ZIP 旁都有同名 `.zip.sha256` sidecar。

來源包維持固定排序與時間戳，不含 `.git`、快取、`dist` 或產生的檢查檔。

驗證：

```bash
unzip -t dist/SJRC-AutoReader-Repo-1.0.1.zip
unzip -t dist/SJRC-AutoReader-Web.zip
unzip -t dist/SJRC-AutoReader-Android.zip
unzip -t dist/SJRC-AutoReader-Windows.zip
python3 scripts/check_repo.py --archive dist/SJRC-AutoReader-Repo-1.0.1.zip
```

## 標籤與發佈

```bash
git tag -a v1.0.1 -m "SJRC AutoReader 1.0.1"
git push origin main
git push origin v1.0.1
```

從標籤建立 GitHub release：

1. 標題：`SJRC AutoReader 1.0.1`
2. 說明：複製 `CHANGELOG.md` 的該節，加上簡短「驗證什麼」區塊。
3. 附加來源 zip、三個平台套件及各自的 `.zip.sha256` sidecar。
5. CI 綠燈後才標為 latest。

## Release notes 模板

```markdown
## 變更
- …

## 驗證
- SHA-256：`…`
- `python3 scripts/check_repo.py` 通過
- 測試：web（Chrome 130）、Android 14、Windows 11 x64

## 已知問題
- …

## 範圍
不包含書籍內容、DRM 金鑰或憑證。
```

## 發佈後

- 更新 `SECURITY.md` 與中文版的「目前版本」表。
- 在 `CHANGELOG.md` 開新的 `## [Unreleased]`。
- 若二進位檔變更，在發佈 commit 更新 `data/source-stats.json`。
- 不要重寫已發佈的標籤。若版本壞了，發新的 patch，並在說明標記舊版撤回。

## 熱修

安全修復可跳過正常 minor 節奏，但仍需要：

- 私下報告或清楚 issue，
- 最小 diff，
- 測試或記錄手動驗證，
- 不暴露使用者的 release note。

## 回滾

保留上一個版本的成品。新版本退化時，指引使用者到上一個標籤，並發佈回滾的
patch。不要用不同二進位檔無聲替換 asset。
