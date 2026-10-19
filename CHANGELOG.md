# Changelog

All notable changes to this project are documented here. The format follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/) and the project uses
[Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [1.0.1] — 2026-10-20

### Changed

- Releases now provide complete per-platform ZIP bundles instead of loose app files.
- The web bundle includes a Python HTTP server and Windows `.bat` launcher; all
  bundles include a guide, licence, and internal checksums.
- README and installation guides explain bundle extraction and verification.

### Added

- Repository checker (`scripts/check_repo.py`) for links, docs parity, data
  consistency, checksums, SVG/XML validity, and accidental secrets
- Deterministic source packaging (`scripts/package_release.py`)
- GitHub Actions CI and tagged release workflow
- Issue forms, expanded pull-request template, Dependabot configuration
- Bilingual architecture, installation, usage, building, testing, security,
  privacy, legal, release, FAQ, glossary, and code-map documents
- Clean endpoint tables (`data/api-endpoints.{csv,json}`), module summary, and
  source statistics

### Changed

- README and Chinese README rewritten with a documentation index, scope limits,
  checksums, and a clearer security/privacy section
- Troubleshooting and reverse-engineering pages expanded with the observed error
  codes and the official-client `end` mismatch

### Fixed

- Corrected the Android wrapper's minimum SDK statement: the wrapper is
  `minSdk 23`; the original SJRC client is `minSdk 26`
- Documented the actual sign-in matrix: the web build is Google-only, the
  Android wrapper is email/password-only, and Windows uses a local helper or a
  pasted token
- Documented the wrapper package (`club.sjrc.autoread`) separately from the
  original package (`club.sjrc.app`)

## [1.0.0] — 2026-10-19

### Added

- Single-file web client with Google sign-in, configurable target, heartbeat,
  page turns, session end, and an in-page log panel
- Android WebView wrapper APK (email/password sign-in only) with an embedded
  copy of the web UI
- Native Windows Win32/WinHTTP client with local OAuth helper, token import,
  registry-backed settings, and static linking
- Decompiled client excerpts for the network interceptors, token manager,
  reading-time view model, request builder, and response entity
- Analysis notes and curated endpoint tables based on the 2.9.0 client reference;
  the full decompiled source tree is not redistributed
- English and Traditional Chinese documentation

### Security

- No backend, telemetry, analytics, or credential collection
- Windows build disables redirects and keeps tokens in the current user's
  registry hive
- Repository contains no tokens, private keys, or decrypted content

## Earlier internal revisions

Before 1.0.0 the project existed as a set of local experiments (web page, APK,
Windows executable, and decompilation notes). They were consolidated into this
repository. No public release or support commitment exists for those revisions.

