# Contributing to SJRC AutoReader

[繁體中文](CONTRIBUTING.zh.md) · **English**

Thanks for taking the time to help. This repository is intentionally small: a
single-file web app, a tiny Android WebView wrapper, a native Windows client, and
the decompiled client excerpts that document the protocol. That shape is a
feature, not an accident.

Please read [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md), [SECURITY.md](SECURITY.md),
and the [scope limits](#scope-limits) before opening a pull request.

## Ways to contribute

- Report a reproducible bug with the [bug form](.github/ISSUE_TEMPLATE/bug_report.yml)
- Improve documentation, translations, diagrams, or examples
- Add a regression test or a checker that catches a real failure
- Reproduce a platform issue and attach logs with **no tokens or personal data**
- Review a pull request, especially for security-sensitive changes

Feature requests are welcome, but the project will not add features that download,
decrypt, store, or redistribute book content, or that bypass authentication or
quotas.

## Scope limits

The project must stay within these boundaries:

1. **No content access.** It calls reading-time and reading-position APIs. It
   does not download or decrypt books, and it does not ship DRM keys.
2. **No credential collection.** There is no server, analytics endpoint, or
   telemetry. Tokens remain in the user's browser storage, Android WebView
   storage, or Windows registry.
3. **No new runtime dependency** without a written proposal. The web build is
   vanilla JS; the Windows build is static C++; the Android wrapper is minimal.
4. **No fabricated API facts.** Protocol changes must be backed by the public
   Swagger document, a captured response with secrets removed, or the decompiled
   client. Say which one.
5. **No licence laundering.** Decompiled smali remains the property of its
   original owner. Do not relicense it or copy it into a differently licensed
   project.

## Development setup

There is nothing to install for the web and docs:

```bash
git clone <your-fork-url> SJRC-AutoReader-Repo
cd SJRC-AutoReader-Repo
python3 scripts/serve_web.py          # http://127.0.0.1:8080
python3 scripts/check_repo.py         # offline docs + data checks
```

Optional tools:

| Tool | Used for |
|---|---|
| Python 3.10+ | checkers, data generation, packaging |
| Node.js 18+ | `node --check` on the extracted web script (CI only) |
| `g++` (MinGW-w64) + `windres` | rebuilding the Windows executable |
| `apksigner`/Android SDK | optional; the checked-in APK is already signed |

No Android SDK is required to read or review the Android wrapper.

## Repository conventions

- The **product is the source** for the web build: `web/SJRC_AutoReader.html`
  is edited directly. Do not introduce a bundler.
- `web/src/README.md`, `phone/src/README.md`, and `pc/src/README.md` explain the
  layout next to each artifact.
- The full decompiled reference tree is not distributed in this repository.
  Keep protocol notes in `data/` and the bilingual documentation.
- Docs are bilingual. Every `docs/en/X.md` needs a matching `docs/zh/X.md` with
  the same set of headings and links. `scripts/check_repo.py` enforces parity.
- Code identifiers and API field names stay in English. Prose may be English or
  Traditional Chinese.
- Dates use ISO 8601 (`2026-10-19`). Times, when needed, include a timezone.
- No emoji-only commits, no generated lockfiles, no `node_modules`.

## Making a change

1. Open an issue for anything larger than a typo or a one-line fix. State the
   problem, the platform, and what you already tried.
2. Fork the repository and create a topic branch:
   `git checkout -b fix/heartbeat-timeout`.
3. Make the smallest change that solves the problem. Keep unrelated formatting
   out of the diff.
4. Update **both** language versions of any affected doc, plus `CHANGELOG.md`
   under `## [Unreleased]`.
5. Run the checks:

   ```bash
   make check
   ```

6. Open a pull request using the template. Describe the user-visible effect,
   how you tested it, and any remaining risk.

## Commit messages

Use a short imperative subject, optionally with a conventional prefix:

```text
fix(web): stop timer when the target is reached
docs(api): clarify that reading-time values are seconds
test(repo): reject docs/en pages without a zh counterpart
```

Keep the subject under 72 characters. Explain *why* in the body when the change
is not obvious. Do not include tokens, account identifiers, or personal data in
commit messages or screenshots.

## Review and merge

- CI must pass (`check`, web syntax, docs parity, link check, data validation).
- One maintainer approval is required. Security-sensitive changes may require a
  second review or a private discussion.
- The maintainer squash-merges by default so the main history stays readable.
- A merged change is not a release. Releases follow
  [docs/en/release.md](docs/en/release.md).

## Reporting security issues

Do **not** open a public issue for a vulnerability, leaked token, or a bypass
that could affect other users. Follow [SECURITY.md](SECURITY.md).

## Licence

By contributing, you agree that your original contribution is licensed under the
MIT licence in [LICENSE](LICENSE). You retain copyright in your contribution.
Third-party and decompiled material is **not** relicensed by this project; see
[NOTICE.md](NOTICE.md).
