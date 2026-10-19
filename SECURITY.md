# Security policy

[繁體中文](SECURITY.zh.md) · **English**

## Supported versions

This project ships three small builds and has no long-term support branches.

| Version | Supported |
|---|---|
| 1.0.x (current) | ✅ |
| < 1.0 | ❌ |

## Reporting a vulnerability

**Do not open a public issue.** Use GitHub's private vulnerability reporting:

1. Go to the repository's **Security** tab.
2. Choose **Advisories → Report a vulnerability**.
3. Include the affected build (web / Android / Windows), version or commit,
   reproduction steps, and the impact you believe it has.

If private reporting is unavailable, open a minimal public issue that says only
"I need a private security contact" — no details, no payload, no token.

You can expect an acknowledgement within **7 days** and a status update within
**30 days**. These are targets, not a contract; this is a volunteer project.

## In scope

- A build sending credentials or tokens to any host other than
  `api.sjrc.club` (or the configured Google identity endpoint for web sign-in)
- A token, refresh token, or verification code written to a log, screenshot,
  error message, URL, or committed file
- A documentation claim that would cause a user to expose a credential
- A build script that executes untrusted input or publishes a mismatched binary
- Supply-chain issues in the repository's own scripts and CI
- A way to make the app perform actions outside its documented reading-time and
  reading-position scope

## Out of scope

- Vulnerabilities in the upstream SJRC service or official app. Report those to
  the service operator.
- Rate limits, account bans, or terms-of-service enforcement.
- The fact that tokens are stored in `localStorage` / `HKCU` without OS-level
  encryption. This is a documented design trade-off, not a vulnerability; the
  threat model is in [docs/en/security.md](docs/en/security.md).
- Denial of service against your own machine or account.
- Missing hardening headers on a local `python3 -m http.server` instance.
- Issues that require an already-compromised device or administrator account.

## Safe harbour

We will not pursue legal action against researchers who:

- Make a good-faith effort to avoid privacy violations, data destruction, and
  service disruption
- Only test against their **own** account and devices
- Give us reasonable time to fix the issue before public disclosure
- Do not access, download, or redistribute other users' data or book content

## What we will never ask for

We will never ask you to send a real access token, refresh token, password,
verification code, private key, or decrypted book. Redact tokens as
`eyJ…<redacted>` and account identifiers as `<account-id>`.

## Security design notes

- No backend, no telemetry, no third-party analytics.
- The web build talks directly to `https://api.sjrc.club/`.
- The Android wrapper loads a local asset and makes the same direct calls.
- The Windows build uses WinHTTP with `followRedirects=false`.
- Release binaries are checked against `SHA256SUMS` and `data/source-stats.json`
  by `scripts/check_repo.py`.

For the full threat model, see [docs/en/security.md](docs/en/security.md).
