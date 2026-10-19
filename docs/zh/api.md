# API 參考

**繁體中文** · [English](../en/api.md)

Base URL：`https://api.sjrc.club/`

權威 schema：<https://api.sjrc.club/swagger-json>（公開，不需要 token）。以下
是本專案依賴的子集，已對照反編譯的 2.9.0 客戶端與實際回應。

> API 不是我們的，可能隨時變更。本頁是 2.9.0 的快照，不是 SJRC 的合約。

## 傳輸

| 項目 | 值 |
|---|---|
| Scheme | 僅 HTTPS |
| Host | `api.sjrc.club` |
| Content type | `application/json` |
| Timeout | 網頁 30 秒；Windows 連線 8 秒／接收 15 秒；官方客戶端 60 秒 |
| Redirect | Windows 停用；網頁用瀏覽器預設，但未觀察到重新導向 |
| 憑證固定 | 本專案與擷取客戶端都沒有 |
| CORS | 網頁版需要 API 允許頁面 origin；Google GIS 另有自己的 origin 白名單 |

應用不會把 token 放在 query string。憑證只經標頭或 JSON body 傳遞。

## 標頭

| 標頭 | 值 | 時機 |
|---|---|---|
| `x-auth-schema` | `CLIENT` 或 `PUBLIC` | 每個請求 |
| `authorization` | `Bearer <accessToken>` | 僅 schema 為 `CLIENT` 且 token 存在 |
| `x-current-platform` | `android` | 每個請求 |
| `x-current-locale` | `zh-HK` | 每個請求 |
| `version` | `2.9.0` | 每個請求 |
| `content-type` | `application/json` | 有 body 的請求 |

`x-auth-schema` 為什麼重要：伺服器有 guard，公開路由若標成 `CLIENT` 會被拒，
客戶端路由若缺少或標成 `PUBLIC` 也會被拒。擷取到的攔截器也以
`schema == "CLIENT"` 作爲是否加 Bearer 的條件。

觀察到的回應：

```text
公開路由 + x-auth-schema: CLIENT  -> 403 { code: 10104, message: "Forbidden resource" }
公開路由 + 沒有 schema            -> 403 同上
客戶端路由 + 沒有 Bearer          -> 401 { code: 10117, ... }
```

## 回應信封

```json
{
  "code": 200,
  "data": {},
  "message": "OK",
  "totalCount": 0,
  "isRefresh": false
}
```

HTTP status 不是唯一訊號。客戶端以 **body 的 `code` 為 200 或 201** 判定成功：

```js
isSucc(j) === (j.code === 200 || j.code === 201)
```

`data` 通常是物件，有時是陣列，`PATCH` 有時沒有。`message` 可能已在地化。

## 認證

### 電郵 + 密碼

```http
POST /v1/public/accounts/users/email/pre-login
Content-Type: application/json
x-auth-schema: PUBLIC

{"email":"user@example.edu","password":"<UTF-8 密碼的 base64>"}
```

可能回應：

```json
{"code":200,"data":{"accessToken":"…","refreshToken":"…"}}
{"code":200,"data":{"ticket":"…"}}
```

若只有 `ticket`，代表帳號需要驗證碼：

```http
POST /v1/public/accounts/users/email/login
{"ticket":"<ticket>","verificationCode":"123456"}
```

### Google

```http
POST /v1/public/accounts/users/google/pre-login
{"providerId":"<Google ID token>","type":"web"}
```

`type` 必須與憑證的 client 類型一致。網頁 GIS 用 `web`；Android 憑證會用
`android`，但本專案無法把 Android package/SHA-1 登記到 SJRC 的 Google 專案。
類型不符通常回 `20002`。

回應形狀與電郵相同：`accessToken` 或 `ticket`，然後：

```http
POST /v1/public/accounts/users/google/login
{"ticket":"<ticket>","verificationCode":"123456"}
```

### 續期

```http
POST /v1/public/accounts/users/token/refresh
x-auth-schema: PUBLIC
{"refreshToken":"<refresh token>"}
```

回傳新的 `accessToken`，通常也有新的 `refreshToken`。`20042` 代表 refresh
token 過期，必須重新登入。`20043` 通常代表 token 屬於較舊的伺服器 session；
清掉後重新登入。

### 發送驗證碼

```http
POST /v1/public/accounts/users/send-verifications
{"type":"email","identify":"user@example.edu"}
→ {"code":200,"data":{"status":true}}
```

這個回應**沒有 ticket**。只有驗證碼不能兌換。Ticket 來自 `pre-login`，代表
密碼／id_token 步驟已成功。這就是使用者先按「發送驗證碼」再輸入時看到
「missing ticket」的原因。

## 本專案使用的端點

| 方法 | 路徑 | Body | 備註 |
|---|---|---|---|
| GET | `/v1/client/reading-time` | — | `totalQuota`、`usedQuota`，單位秒 |
| POST | `/v1/client/reading-time/start` | `{"bookId": 123}` | 回傳 `sessionId` |
| PATCH | `/v1/client/reading-time` | `{"sessionId":"…","time":3}` | `time` 是秒 |
| PATCH | `/v1/client/reading-time/end` | `{"sessionId":"…"}` | 官方送 `bookId`，伺服器回 80009 |
| POST | `/v1/client/reading-pos` | `{"bookId":123,"pos":{...}}` | 形狀見下 |
| GET | `/v2/client/books/{id}` | — | 書名、作者、章節 |
| GET | `/v1/client/bookshelf/books?page=1&limit=50` | — | 優先選書來源 |
| GET | `/v1/client/homepage` | — | 書架太少時的備援 |
| GET | `/v1/client/users/detail` | — | 個人資料；也用來驗證 token |
| POST | `/v1/public/accounts/users/email/pre-login` | `{email,password}` | 密碼 base64 |
| POST | `/v1/public/accounts/users/email/login` | `{ticket,verificationCode}` | |
| POST | `/v1/public/accounts/users/google/pre-login` | `{providerId,type}` | |
| POST | `/v1/public/accounts/users/google/login` | `{ticket,verificationCode}` | |
| POST | `/v1/public/accounts/users/send-verifications` | `{type,identify}` | 無 ticket |
| POST | `/v1/public/accounts/users/token/refresh` | `{refreshToken}` | |

完整清單見 [`data/api-endpoints.csv`](../../data/api-endpoints.csv)：98 個
Retrofit 方法、67 個唯一 method+path。

## 閱讀位置 payload

```json
{
  "bookId": 12345,
  "pos": {
    "chapterId": "3",
    "chapterLocation": 7,
    "position": 0.35
  }
}
```

| 欄位 | 型別 | 意義 |
|---|---|---|
| `chapterId` | string | 官方把章節索引以字串送出 |
| `chapterLocation` | integer | 章節內 1-based 頁碼 |
| `position` | number | 章節內頁碼除以每章頁數，4 位小數 |

本專案用估算分頁，不解析真實版面。伺服器接受這個形狀；數值是近似。

## 單位

`totalQuota`、`usedQuota`、`time` 都是**秒**。

| 人類值 | 原始值 |
|---|---:|
| 分配 8.00 小時 | `totalQuota: 28800` |
| 已讀 3.04 小時 | `usedQuota: 10933` |
| 剩 4.96 小時 | `17867` |
| 一次心跳 | `time: 3` |

`10933 / 3600 = 3.036`。若 UI 寫「分鐘」，就是錯了 60 倍。

## 錯誤碼

| 代碼 | 意義 | 客戶端行為 |
|---|---|---|
| `10102` | DTO 驗證失敗 | 記錄單位／session 提示，計入錯誤 |
| `10104` | `x-auth-schema` 不匹配／禁止 | 修正 schema；不要盲目重試 |
| `10117` | Token 無效 | 續期一次，下個 tick 重試 |
| `10119` | Token 過期 | 續期一次，下個 tick 重試 |
| `20002` | Google ID token 被拒 | 檢查 client 類型／origin |
| `20042` | Refresh token 過期 | 停止；重新登入 |
| `20043` | Session not found | 清掉殘留 refresh token；重新登入 |
| `20045` | 帳號未註冊 | 顯示訊息；不重試 |
| `80008` | 配額用盡 | 停止 |
| `80009` | Reading session 不存在 | end 必須送 `sessionId` |
| `-777777` | 剩餘 ≤ 20% | 警告；可能繼續 |
| `-1001` | 強制登出 | 連續 4 次失敗後停止 |

代碼是整數，可以為負。未知代碼一律視為錯誤；記錄路徑與 HTTP status。

## curl 範例

在 shell 設定變數；不要把真實 token 貼進會提交的腳本。

```bash
BASE=https://api.sjrc.club
TOKEN='<access token>'
COMMON=(-H "x-auth-schema: CLIENT" -H "x-current-platform: android" \
        -H "x-current-locale: zh-HK" -H "version: 2.9.0")

# 配額
curl -sS "$BASE/v1/client/reading-time" "${COMMON[@]}" \
     -H "authorization: Bearer $TOKEN" | jq .

# 開 session
curl -sS -X POST "$BASE/v1/client/reading-time/start" "${COMMON[@]}" \
     -H "authorization: Bearer $TOKEN" -H 'content-type: application/json' \
     -d '{"bookId":12345}' | jq .

# 心跳（time 是秒）
curl -sS -X PATCH "$BASE/v1/client/reading-time" "${COMMON[@]}" \
     -H "authorization: Bearer $TOKEN" -H 'content-type: application/json' \
     -d '{"sessionId":"<session-id>","time":3}' | jq .

# 結束（sessionId，不是 bookId）
curl -sS -X PATCH "$BASE/v1/client/reading-time/end" "${COMMON[@]}" \
     -H "authorization: Bearer $TOKEN" -H 'content-type: application/json' \
     -d '{"sessionId":"<session-id>"}' | jq .
```

公開路由範例：

```bash
curl -sS -X POST "$BASE/v1/public/accounts/users/email/pre-login" \
  -H 'x-auth-schema: PUBLIC' -H 'content-type: application/json' \
  -d '{"email":"user@example.edu","password":"<base64>"}' | jq .
```

## 重試與續期策略

- 網頁：JWT 到期前 90 秒續期，以及收到 10117/10119 時續期。
- Windows：執行中每 150 秒續期，以及收到 10117/10119 時續期。
- 續期失敗對該次執行是致命的；要求使用者重新登入。
- 不要在緊密迴圈重試 10102/10104/80008。修正請求或停止。
- 若服務回傳限流，應退避；本專案除了固定 tick 外沒有內建指數退避。

## 刻意缺席

- 不自動呼叫 `send-verifications`；使用者必須在正式流程中觸發。
- 不嘗試取得 `booksKey`、解密書籍或呼叫下載端點。
- 不嘗試繞過配額、CAPTCHA 或帳號狀態。
- 不做憑證固定，因此本機 TLS 攔截代理可以讀取流量。不要在不受信任的網路上
  執行本應用。
