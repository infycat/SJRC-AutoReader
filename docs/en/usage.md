# Usage

[中文](../zh/usage.md) · English

This page describes a repeatable run: what happens before the first request, what
the app does while it runs, and how to stop without leaving a dangling session.

## Pre-flight checklist

- [ ] You are allowed to automate reading for this account (ask your school).
- [ ] The file hash matches `SHA256SUMS`.
- [ ] The target is realistic: start with `0.1`–`0.5` hours, not `8`.
- [ ] You are on the right account. Check the name/email in the account card.
- [ ] No other client is running against the same account.
- [ ] The device will not sleep or lose network for the whole run.

## Sign in

| Method | Web | Android | Windows |
|---|:---:|:---:|:---:|
| Google identity | ✅ | ❌ | ✅ local helper |
| Email + password | ❌ | ✅ | ❌ |
| Verification code | ✅ | ✅ | ✅ |
| Paste access token | ❌ | ❌ | ✅ |

The login result is an access token plus, usually, a refresh token. A response
that contains only a `ticket` means a second factor is required: enter the code
that was sent to your email. The ticket is one-shot and expires.

## Start a run

1. Open the app and sign in.
2. Confirm the quota card shows a plausible total (for example `8.00 小時`).
3. Set **Target hours** (start small).
4. Leave heartbeat at `3` seconds and page turn at `20` seconds unless you have a
   reason to change them.
5. Optional: tick **Demo mode** for one minute. The counters move, the log says
   demo, and no request is sent.
6. Untick demo and press **Start**.

The app performs this sequence:

```text
GET  /v1/client/reading-time                 baseline quota
GET  /v1/client/bookshelf/books?page=1&limit=50
GET  /v1/client/homepage                     only if the shelf has < 3 books
GET  /v2/client/books/{id}                   title, authors, chapters
POST /v1/client/reading-time/start {bookId}  -> sessionId
POST /v1/client/reading-pos {bookId,pos}     initial estimate
then, every second, tick():
  every hbSec:   PATCH /reading-time {sessionId,time}
  every turnSec: POST /reading-pos, and advance the page
  every tick:    check target / quota / error count
```

## Reading the status

| Indicator | Meaning |
|---|---|
| `running` pill | The scheduler is active |
| Heartbeat dot | A heartbeat is in flight or just completed |
| Page-turn dot | A page turn is in flight |
| `net:` pill | Last request path and outcome |
| Used / total | Server quota in **seconds**, formatted as hours |
| Elapsed | `max(local reported seconds, server used delta)` in hours |
| Remaining | Target minus elapsed |
| Heartbeats / turns / books | Counters for this run |
| Errors | Consecutive heartbeat failures out of 3 |

If the server and local counters disagree, the progress card takes the larger
value. This is deliberate: it avoids showing negative progress after a refresh.

## Stop conditions

The run stops automatically when any of these happens:

| Condition | Automatic action |
|---|---|
| Target reached | `endBook()`, timer cleared |
| `usedQuota > totalQuota` | Stop with quota message |
| Remaining ≤ 20% of total | Warning in the log; the run may continue |
| 4 consecutive heartbeat failures | Stop; mirrors the official forced logout |
| `end` fails after a manual stop | Logged; local session cleared anyway |
| Book finished | `end`, then random next book |
| Page count exhausted | Treated as book finished |

Press **Stop** to end early. The app sends
`PATCH /v1/client/reading-time/end {sessionId}` and clears `sjrc.session`.
Closing the tab without stopping cannot run cleanup; the server session expires
on its own, but the next run may start a new one.

## Logs

The log panel shows timestamps, request paths, response codes, and errors. It
never intentionally prints a token. Before sharing a log:

1. Remove or redact `Bearer …`, `eyJ…`, ticket values, session IDs, emails,
   school names, and book titles.
2. Keep the error code (`10102`, `80009`, `-777777`) and the request path.
3. State the build and version.

Web/Android have **Clear** but no export button. Select the text manually if you
need to keep it. Windows has a read-only log box; the same rule applies.

## Windows local sign-in helper

The Windows build can open a loopback page:

1. Press **開啟 Google 登入頁**.
2. The helper listens on `47821` for the main page and temporarily on `8080` for
   Google (the official client's allowed origin).
3. Sign in; the helper exchanges the Google credential for an SJRC token and
   writes it into the desktop app.
4. The server closes after success or after 2–3 minutes idle.

If the ports are occupied, use **manual import**: paste an access token into the
field and press **手動導入**.

## Android lifecycle

- Keep the app in the foreground. WebView timers are throttled in the background.
- `onStop()` calls `__appPause()` which stops the loop and requests session end.
- The system may kill the process. A stale session is harmless, but the next run
  opens a fresh one.
- The wrapper has no Google sign-in. Use email + password.

## Good habits

- One active client per account. Parallel sessions produce confusing quota
  deltas and may trip rate limits.
- Prefer shorter targets you can verify. Scale up only after a successful 0.1 h
  run.
- Do not run while travelling between networks; a half-open request looks like a
  failure and increments the error counter.
- Check the quota card after the run. It is the only source of truth.
- Log out on shared machines. The token is stored in plain browser storage or
  the registry.

## What the app will not do

- It will not keep running after the browser tab is closed.
- It will not resume an interrupted session with the same `sessionId`.
- It will not bypass a quota, a ban, a CAPTCHA, or a forced password change.
- It will not hide that a client is calling the API; it sends the same headers as
  the captured official client.
