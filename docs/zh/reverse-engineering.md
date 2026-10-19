# SJRC 客戶端逆向分析

[English](../en/reverse-engineering.md) · 繁體中文

本專案所有網路行為都從公開散布的 `club.sjrc.app` 2.9.0 Android 版本推導，並
與公開 Swagger 文件交叉驗證。本頁記錄方法、證據與結論的界線，不只列最後的端點。

> **界線。** 分析是為了互通性與協議理解。儲存庫不含可用的內容金鑰、憑證或
> 使用者資料。反編譯類別中與內容層有關的程式碼仍作為證據保留，但金鑰常數已移除。
> 見 [法律與可接受使用](legal.md) 與 [NOTICE.md](../../NOTICE.md)。

## 分析對象

| 項目 | 值 |
|---|---|
| 套件 | `club.sjrc.app` |
| 版本 | 2.9.0（versionCode 120） |
| 散佈形態 | `.apks` 分卷：base + arm64-v8a + xxhdpi + zh |
| `base.apk` | 116,959,933 bytes |
| DEX 數量 | 6，合計約 53 MB |
| 簽入的 smali 類別 | 4,888 |
| `minSdk` / `targetSdk` | 26 / 35 |
| 混淆 | 部分 —— `club.sjrc.app.*` 與 `com.middle.*` 保留原名 |
| 簽入的參考 | `sjrc-source/smali/`、`AndroidManifest.xml`、`net_endpoints.json` |

參考樹不是完整應用程式：它是理解網路與閱讀迴圈所需類別的完整 smali，加上解碼
manifest 與正規化端點表。它足以稽核本頁主張，但不是可重新散布的 App 重建。

## 研究問題與方法

問題很窄：**客戶端到底要送什麼，伺服器才會計入閱讀時間？官方 App 哪些行為只是
實作細節？**

依可信度使用三個來源：

1. **出貨 APK 的 smali** —— 常數、控制流、欄位名的位元級真相。
2. **公開 Swagger** —— DTO 形狀、驗證、錯誤碼的權威，但不描述 App 的呼叫順序。
3. **實際帳號** —— 只用來確認單位與成功信封。不簽入任何帳號資料、token 或回應 body。

只有至少兩個來源一致，或單一來源明確無歧義（例如 3000 ms 倒數旁的
`const-wide/16 3`），主張才被接受。

```text
SJRC_2.9.0.apks
  └─ 解包
       └─ base.apk
            ├─ aapt2 dump badging/xmltree → AndroidManifest.xml
            ├─ baksmali                   → sjrc-source/smali/   （位元級真相）
            ├─ androguard / jadx          → 可讀 Java            （失真，未附）
            ├─ strings / grep             → 端點字串
            └─ swagger-json               → DTO 與錯誤碼交叉比對
```

## 還原 manifest 與 SDK

`aapt2 dump badging base.apk` 不執行程式就能取得套件、版本、SDK 等級與啟動
activity。解碼後的 manifest 放在 `sjrc-source/AndroidManifest.xml`，讓權限與
元件名稱可以用純文字檢視。

要分清楚兩個 App：

| App | 套件 | 版本 | minSdk | 用途 |
|---|---|---|---:|---|
| 官方客戶端 | `club.sjrc.app` | 2.9.0（120） | 26 | 協議參考 |
| 本專案 Android 外殼 | `club.sjrc.autoread` | 1.0（1） | 23 | WebView 外殼 |

不要把兩份 manifest 混在一起。外殼的 `minSdk 23` 是本專案的屬性，不是 SJRC
支援版本的證據。

## 為什麼以 smali 為準

反編譯 Java 比較好讀，但會失去泛型，並誤譯 Kotlin 的 `suspend` 狀態機與 inline
函式。做協議工作時，小細節比可讀性重要。`time` 欄位就是典型案例：

```smali
# BookReadViewModel.readTimeUsed()
iget v0, p0, ->errorTime:I
const-wide/16 v1, 3          ; 基礎值
if-lez v0, +004h             ; errorTime <= 0 時跳過乘法
int-to-long v3, v0
mul-long/2addr v1, v3        ; v1 = 3 * errorTime
```

Java 檢視可能只看到輔助函式或 `3L`；smali 顯示這是**以 3 秒為基礎**的值，且
連續失敗會乘上去。這就是客戶端送 `time: 3` 而不是 `3000`、也不是時間戳的原因。

## 客戶端架構

```text
UI          WelcomeLoginActivity · ReadActivity · MagazineActivity
ViewModel   BookReadViewModel              ← 心跳核心
Repository  HttpRepo（單例）
Network     Retrofit + OkHttp
            ParameterInterceptor            標頭注入
            TokenExpireInterceptor          token 錯誤時續期再重試
            ExceptionCatchInterceptor
Model       UseTimeEntity · ReadPosRequest · NetResponse
Crypto      內容層（下方描述，本專案不實作）
```

### 標頭：哪些必要、哪些只是實作細節

`ParameterInterceptor` 對每個請求注入同一組標頭。重要的是：

| 標頭 | 值 | 為什麼重要 |
|---|---|---|
| `version` | `2.9.0` | 伺服器端相容性閘門 |
| `x-current-platform` | `android` | 選擇客戶端契約 |
| `x-current-locale` | `zh-HK`（本專案） | 官方由 `LanguageManage` 產生；伺服器接受固定值 |
| `x-auth-schema` | `CLIENT` 或 `PUBLIC` | 決定套用哪套認證規則 |
| `authorization` | `Bearer <accessToken>` | **只**用於 `CLIENT` 且 token 非空 |

`x-auth-schema` 不是選填。客戶端路由漏送、或把 token 送到 `PUBLIC` 路由，會得到
`10104`，而不是一般 401。這是讓三個客戶端可行的第一批發現之一。

### 兩套網路風格並存

```java
// 1. Map 組裝 —— 完全沒有型別檢查
new BodyBuilder().put("time", boxLong(3)).toBody()

// 2. 具型別的 DTO
new ReadPosRequest(bookId, new PosEntity(...))
```

`BodyBuilder` 就是 `HashMap`，用 Gson 序列化。欄位名或單位寫錯，編譯期不會發現，
只在執行期被伺服器擋（`10102`）。具型別 DTO 比較安全，但官方 App 沒有統一
使用。兩者衝突時，形狀以 Swagger DTO 為準，App 實際送出的內容以 smali 為準。

## 心跳狀態機

這是官方客戶端的核心，從 `BookReadViewModel` 還原：

```text
handReadTimeStart()
  POST /v1/client/reading-time/start { bookId }
    成功 → bookSessionId = data.sessionId, errorTime = 0

startCountDown()
  setMillisInFuture(9999999000)
  setCountDownInterval(3000)
  tick → handReadTimeUse()

handReadTimeUse()
  time = (errorTime > 0) ? 3 * errorTime : 3
  PATCH /v1/client/reading-time { sessionId, time }
    成功 → 檢查額度
        !isUserQuotaValid()      → readStatus 2,  EventBus( 80008 )
        isRemainTimeLessThan20() → readStatus 13, EventBus(-777777 )
    失敗 → errorTime++
        errorTime > 3 → EventBus(-1001) 強制登出

handReadTimeEnd()
  PATCH /v1/client/reading-time/end { bookId }        ← 官方客戶端
```

額度判斷：

```kotlin
isUserQuotaValid()       = totalQuota >= usedQuota
isRemainTimeLessThan20() = (totalQuota - usedQuota) <= totalQuota * 0.2f
```

第二個名字有誤導性：它是**比例**檢查，不是「剩 20 分鐘」。即使還剩好幾小時，
只要剩餘 ≤ 20% 就會停。

`end` 的參數錯位是官方客戶端的真實缺陷：Swagger 的 `EndReadingDto` 需要
`sessionId`，App 卻送 `bookId`，伺服器回 `80009`（找不到閱讀 session）。本專案
刻意送 `sessionId`，並記錄差異，而不是照抄 bug。

## 請求管線

```text
呼叫端
  → Retrofit
  → ParameterInterceptor
      若 x-auth-schema == "CLIENT" 且 token 非空
          authorization: Bearer <token>
      無條件加
          x-current-locale: zh-HK
          x-current-platform: android
          version: 2.9.0
  → OkHttp   connect/read/write 60 秒 · followRedirects=false · 無 CertificatePinner
  → api.sjrc.club
  → NetResponse{ code, data, message, totalCount, isRefresh }
       isSucc() == (code == 200 || code == 201)
```

兩個容易漏掉的細節：

- HTTP 狀態不決定成功。`200 OK` 的信封可能帶著 `{code: 80009}`，客戶端必須讀
  `code`。
- Token 續期由 `TokenExpireInterceptor` 把關。續期失敗時 App 強制重新登入，不會
  無限重試；`20042` 與 `20043` 通常就是執行結束的代碼。

## 還原端點表

Retrofit 註解掛在介面方法上。簽入的 `net_endpoints.json` 是機器抽取的 98 個
方法；`data/` 正規化成 CSV 與 JSON，共 67 個唯一 `METHOD + path`。抽取保留
所屬類別，讓主張可以追回 smali。

讀表時要注意三個慣例：

- 執行期才給 `@Url` 的方法沒有字面路徑；正規化列使用 `(dynamic @Url)`。
- 路徑參數保留 `{id}`，不會填範例值。
- 出現在表裡不代表本專案會呼叫。實際使用的只有
  [API 參考](api.md#本專案使用的端點) 列出的路由。

重現原始端點搜尋：

```bash
grep -rn "reading-time" sjrc-source/smali | head
grep -rn "reading-pos" sjrc-source/smali | head
grep -rn "accounts/users" sjrc-source/smali | head
```

端點字串在 DEX 常數池，可能不在你正在讀的方法體內。請沿著持有 `@Url` 註解的
欄位追。

## 驗證了什麼、怎麼驗

| 主張 | 證據 | 交叉檢查 |
|---|---|---|
| `time` 與額度是秒 | `readTimeUsed()` 基礎 3；`setCountDownInterval(3000)`；實際回應如 10933 ≈ 3.04 小時 | 比較已知心跳次數前後的 `usedQuota` |
| `end` 需要 `sessionId` | Swagger `EndReadingDto`；官方送 `bookId` 得到 `80009` | 路由形狀與錯誤文字 |
| 驗證碼需要 ticket | `send-verifications` 只回 `{status:true}`，沒有 ticket；`email/login` 要 `{ticket, verificationCode}` | 沒有 pre-login 的驗證碼會得到 “Missing ticket” |
| Google 不能在 WebView 內登入 | Google OAuth 政策回 `disallowed_useragent` | Android 外殼只提供電郵密碼 |
| 信封成功是 200/201 | smali 的 `NetResponse.isSucc()` | 實際 `{code:200}` 回應 |

刻意不驗證：內容加密的端到端流程、別人的帳號、伺服器內部，以及平台是否允許
自動化。那些超出本專案的範圍與權限。

## 內容加密（僅描述，金鑰已移除）

官方客戶端有內容保護層：

```text
檔案層   AES-192-CBC，金鑰硬編碼在 APK（每檔一個 IV）
金鑰層   RSA-2048 OAEP-SHA256
         GET /v1/client/booksKey?bookId=N → { key: <RSA 密文>, url }
         用 APK 內嵌的 RSA 私鑰解密
```

記錄這段只是解釋官方 App 為何能顯示受保護書籍、以及本專案為何不做。本公開鏡像
已把內嵌的 AES 金鑰、CBC IV 與 RSA 私鑰替換為佔位符，因此簽入的反編譯類別無法
解密受保護內容。三個客戶端從不實作或呼叫這一層，也從不請求 `booksKey`。加入可
用的內容存取路徑會讓專案從協議互通跨進規避，超出範圍。見
[法律](legal.md#不繞過-drm)。

### 金鑰素材的移除

`sjrc-source/smali/` 下有五個檔案帶有 `SECURITY REDACTION` 標記。這棵樹與原始 APK
並非逐位元組相同，不能用來重建官方客戶端，也不含可用金鑰素材。
`data/source-stats.json` 記錄了移除內容。原始 APK 並未隨本儲存庫散布。

## 瀏覽源碼

```bash
# 找類別
find sjrc-source/smali -path "*BookReadViewModel*"

# 列出方法
grep -n "^\.method" \
  sjrc-source/smali/club/sjrc/app/business/book/viewModel/BookReadViewModel.smali

# 端點字串
grep -rn "reading-time" sjrc-source/smali | head
```

| 路徑 | 內容 |
|---|---|
| `club/sjrc/app/net/` | Retrofit service、interceptor、entity |
| `club/sjrc/app/business/book/viewModel/` | 心跳狀態機 |
| `club/sjrc/app/business/login2/` | 登入流程 |
| `club/sjrc/app/manager/` | `TokenManager`、`LanguageManage` |
| `com/middle/network/` | `BodyBuilder`、`NetResponse` |
| `com/skytree/epub/` | 分頁引擎 —— 頁數在這裡算 |

`phone/src/*.smali` 的節錄是給不想走完整棵樹的審閱者的捷徑。它們複製自同一個
2.9.0 建置，未經修改。

## 限制與維護

- 這是 2.9.0 的快照。未來 App 版本可能改標頭、單位或路由；假設表格仍適用之前，
  請重新推導。
- 混淆只是部分，名稱有用但不保證穩定。
- 實際 API 是第三方服務，可能限流、改驗證或消失。本專案沒有後端備援。
- CI 不連真實 API，只檢查連結、資料一致性、語法與秘密外洩。

## 聲明

公開散布 APK 的反編譯輸出，僅供互通性研究。著作權屬原權利人。不含金鑰、憑證、
token 或個資。如需移除，請開 issue。
