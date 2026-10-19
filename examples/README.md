# HTTP request examples

Minimal, copy-pasteable requests for inspecting the protocol with a REST client
that understands the `.http` format (VS Code REST Client, IntelliJ HTTP Client,
JetBrains Gateway, or `httpyac`).

> **Use a test account and read this first.** These requests talk to the real
> service. `reading-time/start` creates a server-side session, the heartbeat adds
> reading time, and `end` closes the session. Do not run them against an account
> you cannot afford to change. Never commit a filled-in file.

## Setup

1. Copy `requests.http` outside the repository, or keep it local and uncommitted.
2. Replace `@token` with an access token from a client you are authorised to use.
3. Replace `@bookId` and `@sessionId` with values from your own account.
4. Send requests one at a time. Read the response envelope (`code`, `message`),
   not just the HTTP status.

The API base is `https://api.sjrc.club/`. Every client route sends
`x-auth-schema: CLIENT`; login and refresh routes use `PUBLIC`. The access token
is sent as `authorization: Bearer <token>` only on `CLIENT` routes.

## Side effects

| Request | Side effect |
|---|---|
| `GET /v1/client/reading-time` | Read-only |
| `GET /v1/client/users/detail` | Read-only |
| `POST /v1/client/reading-time/start` | Creates a reading session |
| `PATCH /v1/client/reading-time` | Adds `time` seconds to the session |
| `POST /v1/client/reading-pos` | Reports a page position (read-only metadata) |
| `PATCH /v1/client/reading-time/end` | Closes the session |
| `POST /v1/public/accounts/users/token/refresh` | Rotates tokens; the old refresh token may stop working |

## Field reminders

- `time` and `totalQuota` / `usedQuota` are **seconds**. `3` is three seconds;
  `28800` is eight hours.
- Heartbeat needs `sessionId`. `end` needs `sessionId`, **not** `bookId`.
- Success is `code == 200 || code == 201` in the JSON envelope.
- `80009` on `end` normally means the wrong identifier or a dead session.
- A verification code is useless without the `ticket` returned by `pre-login`;
  see [API reference](../docs/en/api.md#authentication).

Do not paste real tokens or account data into issues. See
[Privacy](../docs/en/privacy.md) and [SECURITY.md](../SECURITY.md).
