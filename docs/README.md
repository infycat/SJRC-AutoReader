# 文件索引

本儲存庫預設使用繁體中文。請從以下常用頁面開始；完整文件清單與英文版本也列於下方。

- [快速開始](zh/getting-started.md) · [安裝](zh/installation.md) · [設定](zh/configuration.md) · [使用](zh/usage.md)
- [架構](zh/architecture.md) · [API 參考](zh/api.md) · [程式碼地圖](zh/code-map.md) · [逆向工程](zh/reverse-engineering.md)
- [建置](zh/building.md) · [測試](zh/testing.md) · [發佈流程](zh/release.md)
- [安全](zh/security.md) · [隱私](zh/privacy.md) · [法律](zh/legal.md)
- [疑難排解](zh/troubleshooting.md) · [常見問題](zh/faq.md) · [術語表](zh/glossary.md)

---

# Documentation

**English** · [繁體中文](#文件)

The same documentation exists in English (`docs/en/`) and Traditional Chinese
(`docs/zh/`). Every page keeps the same headings and links; `make check` enforces
that parity.

## Start here

| # | Page | What it answers |
|---:|---|---|
| 1 | [Getting started](en/getting-started.md) · [中文](zh/getting-started.md) | What is this, is it safe, first run in 5 minutes |
| 2 | [Installation](en/installation.md) · [中文](zh/installation.md) | Per-platform install, checksums, uninstall |
| 3 | [Configuration](en/configuration.md) · [中文](zh/configuration.md) | Every parameter, default, storage key |
| 4 | [Usage](en/usage.md) · [中文](zh/usage.md) | The full run loop, logs, stopping, good habits |

## Understand the system

| # | Page | What it answers |
|---:|---|---|
| 5 | [Architecture](en/architecture.md) · [中文](zh/architecture.md) | Components, state machine, data flow, design choices |
| 6 | [API reference](en/api.md) · [中文](zh/api.md) | Headers, endpoints, DTOs, units, auth, error codes |
| 7 | [Code map](en/code-map.md) · [中文](zh/code-map.md) | Where each behaviour lives in the source |
| 8 | [Reverse engineering](en/reverse-engineering.md) · [中文](zh/reverse-engineering.md) | How the protocol was recovered and verified |

## Build and verify

| # | Page | What it answers |
|---:|---|---|
| 9 | [Building](en/building.md) · [中文](zh/building.md) | Rebuild web, APK, and EXE |
| 10 | [Testing](en/testing.md) · [中文](zh/testing.md) | Static checks, mock tests, manual matrix |
| 11 | [Release process](en/release.md) · [中文](zh/release.md) | Versioning, packaging, checksums, tags |

## Trust and boundaries

| # | Page | What it answers |
|---:|---|---|
| 12 | [Security](en/security.md) · [中文](zh/security.md) | Threat model, token storage, what is in scope |
| 13 | [Privacy](en/privacy.md) · [中文](zh/privacy.md) | What leaves the device, what is stored, retention |
| 14 | [Legal](en/legal.md) · [中文](zh/legal.md) | Licence scope, disclaimers, takedown, acceptable use |

## Help

| # | Page | What it answers |
|---:|---|---|
| 15 | [Troubleshooting](en/troubleshooting.md) · [中文](zh/troubleshooting.md) | Symptom → cause → fix, with exact error codes |
| 16 | [FAQ](en/faq.md) · [中文](zh/faq.md) | Short answers to the questions we see most |
| 17 | [Glossary](en/glossary.md) · [中文](zh/glossary.md) | Heartbeat, session, ticket, quota, smali, DEX |

## Suggested reading paths

- **Just want to run it:** getting started → installation → troubleshooting
- **Want to change the code:** code map → architecture → building → testing
- **Want to audit it:** security → privacy → API reference → source in `web/` and `pc/`
- **Want to understand the official client:** reverse engineering → API reference → `sjrc-source/README.md`

## Conventions

- Values marked **seconds** are seconds. `28800` is 8 hours, not 8 minutes.
- `club.sjrc.app` is the official client. `club.sjrc.autoread` is this Android wrapper.
- “Official client” means SJRC 2.9.0 (versionCode 120), not this project.
- Commands assume the repository root unless a page says otherwise.
- Relative links are checked offline; external links are not fetched in CI.

---

# 文件

[English](#documentation) · **繁體中文**

同樣的文件存在於英文（`docs/en/`）與繁體中文（`docs/zh/`）。每頁保持相同的標題與
連結；`make check` 會檢查兩邊是否對齊。

## 從這裡開始

| # | 頁面 | 回答什麼 |
|---:|---|---|
| 1 | [快速開始](zh/getting-started.md) · [English](en/getting-started.md) | 這是什麼、是否安全、5 分鐘首次執行 |
| 2 | [安裝](zh/installation.md) · [English](en/installation.md) | 各平台安裝、校驗碼、卸載 |
| 3 | [設定](zh/configuration.md) · [English](en/configuration.md) | 每個參數、預設值、儲存位置 |
| 4 | [使用](zh/usage.md) · [English](en/usage.md) | 完整執行流程、日誌、停止、好習慣 |

## 理解系統

| # | 頁面 | 回答什麼 |
|---:|---|---|
| 5 | [架構](zh/architecture.md) · [English](en/architecture.md) | 元件、狀態機、資料流、設計取捨 |
| 6 | [API 參考](zh/api.md) · [English](en/api.md) | 標頭、端點、DTO、單位、認證、錯誤碼 |
| 7 | [程式碼地圖](zh/code-map.md) · [English](en/code-map.md) | 每個行為在原始碼的哪裡 |
| 8 | [逆向工程](zh/reverse-engineering.md) · [English](en/reverse-engineering.md) | 協議如何被還原與驗證 |

## 建置與驗證

| # | 頁面 | 回答什麼 |
|---:|---|---|
| 9 | [建置](zh/building.md) · [English](en/building.md) | 重建網頁、APK、EXE |
| 10 | [測試](zh/testing.md) · [English](en/testing.md) | 靜態檢查、mock 測試、手動矩陣 |
| 11 | [發佈流程](zh/release.md) · [English](en/release.md) | 版本、打包、校驗碼、標籤 |

## 信任與界線

| # | 頁面 | 回答什麼 |
|---:|---|---|
| 12 | [安全](zh/security.md) · [English](en/security.md) | 威脅模型、token 儲存、範圍 |
| 13 | [隱私](zh/privacy.md) · [English](en/privacy.md) | 什麼離開裝置、存了什麼、保留多久 |
| 14 | [法律](zh/legal.md) · [English](en/legal.md) | 授權範圍、免責、下架、可接受使用 |

## 求助

| # | 頁面 | 回答什麼 |
|---:|---|---|
| 15 | [疑難排解](zh/troubleshooting.md) · [English](en/troubleshooting.md) | 症狀 → 原因 → 修法，含確切錯誤碼 |
| 16 | [常見問題](zh/faq.md) · [English](en/faq.md) | 最常被問的問題與短答 |
| 17 | [術語表](zh/glossary.md) · [English](en/glossary.md) | 心跳、session、ticket、配額、smali、DEX |

## 建議閱讀順序

- **只想執行**：快速開始 → 安裝 → 疑難排解
- **想改程式**：程式碼地圖 → 架構 → 建置 → 測試
- **想審核**：安全 → 隱私 → API 參考 → `web/` 與 `pc/` 原始碼
- **想理解官方客戶端**：逆向工程 → API 參考 → `sjrc-source/README.md`

## 慣例

- 標為**秒**的值就是秒。`28800` 是 8 小時，不是 8 分鐘。
- `club.sjrc.app` 是官方客戶端；`club.sjrc.autoread` 是本專案的 Android 外殼。
- 「官方客戶端」指 SJRC 2.9.0（versionCode 120），不是本專案。
- 指令若無特別說明，均在儲存庫根目錄執行。
- 相對連結由 CI 離線檢查；外部連結不在 CI 抓取。
