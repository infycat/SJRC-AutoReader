# Roadmap

[繁體中文](ROADMAP.zh.md) · **English**

This is a scope-limited interoperability project. The roadmap is intentionally
short, and some items may never ship.

## Current — 1.0.x

- [x] Three builds: single-file web app, Android WebView wrapper, native Windows
- [x] Reading-time heartbeat with target stop and session end
- [x] Reading-position reporting on page turns
- [x] Email/password and verification-code sign-in on every build
- [x] Bilingual documentation, protocol write-up, decompiled excerpts
- [x] Offline repository checker and reproducible source archive

## Next

- [ ] Automated protocol smoke tests against a mock server (no live account)
- [ ] Reproducible Windows build in CI (cross-compile with MinGW-w64)
- [ ] Accessibility pass on the web app (keyboard order, focus states, contrast)
- [ ] More screenshots and a short animated demo (no tokens visible)
- [ ] Translation review for `zh-HK` terminology
- [ ] Signed release checksums published as a GitHub release asset
- [ ] Optional: a minimal read-only dashboard for local session history

## Explicit non-goals

- Downloading, decrypting, storing, or sharing book content
- Shipping DRM keys, RSA private keys, or per-book keys
- Bypassing authentication, quotas, or access controls
- A hosted service, account pooling, or credential collection
- iOS, macOS, or Linux GUI builds
- Evading detection by the upstream service

## How items are chosen

An item moves to "Next" when it has a clear user benefit, a maintainable design,
and no conflict with the non-goals. Documentation and test improvements usually
come first because they make every other change safer.

See [GOVERNANCE.md](GOVERNANCE.md) for decision rules and
[docs/en/release.md](docs/en/release.md) for the release process.
