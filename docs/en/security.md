# Security

[中文](../zh/security.md) · English

## Scope and non-goals

This is a local client that talks to a remote reading service. It is **not** a
security boundary, a secrets manager, or a hardened browser. The security goal is
narrow: keep the user's token from leaking accidentally, make every network
destination auditable, and avoid adding a backend that could become a target.

## Assets

| Asset | Sensitivity | Where it lives |
|---|---|---|
| Password | High | Only in the login form; sent to the API; never stored by this project |
| Google ID token | High | In memory during sign-in; sent to the API |
| Access token | High (bearer) | WebView/localStorage or `HKCU` |
| Refresh token | High | Same |
| Verification ticket | Medium | `sessionStorage` or memory; one-shot |
| Account profile | Medium | Rendered in the UI; not persisted by this project |
| Reading time/position | Low–medium | Sent to the API; local counters |
| Logs | Variable | DOM / Windows edit control; may contain book titles |

Book content and DRM keys are **not** assets of this project because it never
handles them.

## Adversaries

| Adversary | Capability | Mitigation | Residual risk |
|---|---|---|---|
| Another local user | Reads browser profile or registry | OS account separation | Token is plaintext at rest |
| Malware on the device | Reads process memory, storage, keystrokes | OS security | Full compromise; out of scope |
| Network attacker | Intercepts or redirects traffic | TLS verification; no redirects on Windows | No certificate pinning; a trusted MITM proxy can read traffic |
| Malicious dependency | Executes in build or runtime | No third-party client runtime deps; CI reviews | Toolchain compromise; out of scope |
| Malicious contribution | Adds exfiltration code | Review; `check_repo.py` secret scan | A determined contributor can hide logic |
| Upstream service | Sees requests, may log them | Only send what is necessary | Upstream logging/retention is outside our control |
| Shoulder surfer | Reads the screen | Tokens are not displayed after entry | Logs can show book titles |

## Controls

### Network

- HTTPS only; the base URL is hard-coded.
- The Windows client sets `followRedirects=false` so a 30x response cannot move
  the Bearer header to another host.
- The web client uses `credentials: 'omit'` and does not send cookies.
- The API host is `api.sjrc.club`. Google sign-in additionally contacts
  `accounts.google.com` and `www.gstatic.com`.
- No certificate pinning. This avoids breaking on certificate rotation, at the
  cost of trusting the device's CA store.

### Credential handling

- The password is used for one `pre-login` request and is not written to storage.
- The access token is stored in `localStorage` (web/Android) or the registry
  (Windows), both plaintext at rest.
- The token is never placed in a URL or a log line by design.
- The refresh token is only sent to `token/refresh`.
- Logout removes the token and refresh token from storage.
- Closing the browser/Android app does not necessarily clear storage; use logout
  on a shared device.

### Input and output

- The web log escapes server text with `escapeHtml()` before inserting it.
- Book titles and author names are written with `textContent` in the UI.
- Cover URLs come from the API and are assigned to `img.src`; a hostile API
  response could make the browser fetch an arbitrary image URL. This is a
  metadata privacy issue, not a token leak.
- The Windows client parses JSON with a small hand-written parser. It is not
  hardened against adversarial input beyond basic bounds; do not point it at an
  untrusted server.

### Build and release

- `SHA256SUMS` binds the committed artifacts.
- `scripts/check_repo.py` rejects JWT-like strings, private-key blocks, and
  common token patterns in text files.
- CI runs the checker and the web syntax check on every push.
- Release binaries are attached to tags, not silently replaced.
- The Android APK is signed with a project key. The key is not in the repository.

## What is deliberately not protected

- Token encryption at rest.
- Screenshot/recording protection.
- Root/jailbreak detection.
- Anti-debugging or obfuscation.
- Certificate pinning.
- Server-side attestation.

These are not bugs; they are out of scope for a small local client.

## Secure usage checklist

- [ ] Verify the SHA-256 before running a downloaded binary.
- [ ] Use a dedicated browser profile or Windows account if the device is shared.
- [ ] Do not paste a token into a chat, issue, screenshot, or URL.
- [ ] Log out and clear storage before handing the device to someone else.
- [ ] Do not run on a network with a TLS-intercepting proxy you do not trust.
- [ ] Keep the OS and browser updated.
- [ ] Treat any request to “send your token to verify” as a scam. This project
  has no support desk that needs it.

## Reporting a vulnerability

See [SECURITY.md](../../SECURITY.md). Do not open a public issue with a token,
payload, or exploit. Redact values as `eyJ…<redacted>`.

## Threat model changes

If a future version adds a backend, telemetry, a new host, or credential sharing,
this page must be updated in the same pull request. A change that weakens the
model without a documented reason should be rejected.
