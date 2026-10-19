# Testing

[中文](../zh/testing.md) · English

## What CI runs

```bash
make check
```

`scripts/check_repo.py` performs offline checks:

| Check | What it catches |
|---|---|
| Required files | Missing README, licence, docs, data |
| Relative links | Broken `docs/en` ↔ `docs/zh` links, missing images |
| Docs parity | A page exists in one language but not the other, or heading counts differ |
| JSON validity | Broken `data/*.json`, endpoint fields |
| CSV shape | Endpoint rows have the expected columns |
| SHA-256 | Committed artifacts match `SHA256SUMS` and `data/source-stats.json` |
| SVG/XML | Malformed diagrams |
| Accidental secrets | `eyJ…` JWT-like strings, `-----BEGIN … PRIVATE KEY-----`, obvious tokens |
| Forbidden paths | `.git`, caches, editor backups |

CI also runs `node --check` on the web app's inline script and validates the
workflow YAML. It does **not** call the live API.

## Local checks

```bash
python3 scripts/check_repo.py
python3 -m compileall -q scripts
```

The checker exits non-zero on failure and prints the exact path/line.

## Web syntax check

```bash
python3 - <<'PY'
import re
s = open('web/SJRC_AutoReader.html', encoding='utf-8').read()
open('.web-check.js','w',encoding='utf-8').write(re.search(r'<script>(.*)</script>', s, re.S).group(1))
PY
node --check .web-check.js && rm .web-check.js
```

## Static checks for the single-file app

There is no test framework. Keep the following invariants in review:

- `fetch(` only targets `BASE` (search for `fetch(`).
- No token is interpolated into a log line or URL.
- `localStorage.setItem` only writes the four documented keys.
- `innerHTML` is used for static templates, not for server-supplied text. The log
  escapes with `escapeHtml()`.
- Every request sets `x-auth-schema` explicitly.
- `time` is seconds in the heartbeat body.
- `end` sends `sessionId`.

## Manual test matrix

Run these before a release. Mark the platform and build hash.

| # | Scenario | Expected |
|---:|---|---|
| 1 | Web served over HTTP, Google login | Account card populates |
| 2 | Web opened via `file://` | Google button fails with a clear origin error; demo mode still works |
| 3 | Google login with registered origin | Button returns a credential, token appears |
| 4 | Google login with unregistered origin | Error is visible; no request is sent |
| 5 | Demo mode, target 0.1 h | Counters advance, log says demo, no HTTP requests |
| 6 | Real run, target 0.1 h | At least one start, several heartbeats, at least one position, one end |
| 7 | Stop manually | End request sent, timer cleared, buttons reset |
| 8 | Target reached | Automatic stop and end |
| 9 | Invalid token | 10117 path, refresh attempt, then a clear stop |
| 10 | Network offline mid-run | Request error, error counter, stop after 4 |
| 11 | `time` accidentally in ms (test build) | 10102 logged with unit hint |
| 12 | End with `bookId` (test build) | 80009 logged; `sessionId` variant succeeds |
| 13 | Android install over official app | Must fail; packages differ |
| 14 | Android blank page | Check WebView flags; checked-in APK loads |
| 15 | Android background for 5 min | Timer may throttle; no crash |
| 16 | Windows SmartScreen | Warning shown; hash verified before running |
| 17 | Windows helper with port 8080 occupied | Clear error; manual token import works |
| 18 | Windows close during run | Worker stops and sends end |
| 19 | Very long run (1 h+) | Memory stable; log capped in web |
| 20 | Token refresh near expiry | No heartbeat failure is counted |

## Mock-server tests (recommended next step)

The project has no live-account test. A local mock is the right next investment:

```text
POST /v1/client/reading-time/start     -> {code:200,data:{sessionId:"s1"}}
PATCH /v1/client/reading-time           -> {code:200,data:{totalQuota:28800,usedQuota:3}}
POST /v1/client/reading-pos             -> {code:200}
PATCH /v1/client/reading-time/end       -> {code:200}
```

Ideas for assertions:

- The heartbeat body contains `time: 3`, not `3000`.
- The end body contains `sessionId`, not `bookId`.
- A 10117 response triggers exactly one refresh, then a retry.
- Four consecutive failures stop the loop.
- A target of 0 is rejected before any request.

Do not point automated tests at the real service with a real account.

## Release smoke test

1. Verify `SHA256SUMS` on a clean checkout.
2. Run the checker.
3. Open the web app from a fresh browser profile and do a 0.1 h demo.
4. Install the APK on a clean Android profile; verify the package name and
   signature; do a demo run.
5. Run the EXE on a clean Windows VM; verify it starts without DLLs; do a demo
   run.
6. Confirm no token appears in `git status`, screenshots, or logs.

## What is not tested

- The live upstream API's future behaviour
- Google Cloud configuration
- Every Android OEM's WebView quirks
- Windows without a network
- Non-x64 Windows
