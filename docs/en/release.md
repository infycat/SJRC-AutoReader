# Release process

[中文](../zh/release.md) · English

## Versioning

The project uses semantic versioning for its own code:

- **MAJOR** — a change users must act on (storage format, login flow, package name)
- **MINOR** — new behaviour that stays compatible
- **PATCH** — docs, tests, internal fixes

The upstream SJRC version (`2.9.0`) is a protocol target, not this project's
version. Both numbers appear in the docs; do not confuse them.

## Pre-release checklist

- [ ] `make check` passes on a clean checkout.
- [ ] `CHANGELOG.md` has an entry under the new version, with the date.
- [ ] `README.en.md` and `README.zh.md` agree on features and links.
- [ ] `docs/en` and `docs/zh` have the same page set and headings.
- [ ] `SHA256SUMS` matches the committed binaries.
- [ ] `data/source-stats.json` matches sizes and hashes.
- [ ] No token, private key, or personal data in `git status` or screenshots.
- [ ] The three builds were smoke-tested on their target platforms.
- [ ] `CITATION.cff` version/date updated (if you cite releases).
- [ ] No `[Unreleased]` content is silently dropped.

## Build the archive

```bash
python3 scripts/package_release.py --version 1.0.1
python3 scripts/package_downloads.py
```

The scripts create a source archive and three platform bundles:

- `SJRC-AutoReader-Repo-1.0.1.zip`: complete repository snapshot.
- `SJRC-AutoReader-Web.zip`: HTML, local HTTP server, Windows `.bat` launcher, guide, licence, and internal checksums.
- `SJRC-AutoReader-Android.zip`: APK, install guide, licence, and internal checksums.
- `SJRC-AutoReader-Windows.zip`: EXE, usage guide, licence, and internal checksums.
- Each ZIP has a matching `.zip.sha256` sidecar.

The source archive has deterministic ordering and timestamps and excludes `.git`,
caches, `dist`, and generated check files.

Verify the archive:

```bash
unzip -t dist/SJRC-AutoReader-Repo-1.0.1.zip
unzip -t dist/SJRC-AutoReader-Web.zip
unzip -t dist/SJRC-AutoReader-Android.zip
unzip -t dist/SJRC-AutoReader-Windows.zip
python3 scripts/check_repo.py --archive dist/SJRC-AutoReader-Repo-1.0.1.zip
```

## Tag and release

```bash
git tag -a v1.0.1 -m "SJRC AutoReader 1.0.1"
git push origin main
git push origin v1.0.1
```

Create a GitHub release from the tag:

1. Title: `SJRC AutoReader 1.0.1`
2. Notes: copy the `CHANGELOG.md` section, plus a short “what to verify” block.
3. Attach the source zip, three platform bundles, and each `.zip.sha256` sidecar.
4. Mark as “latest” only after CI is green.

## Release notes template

```markdown
## What changed
- …

## Verify
- SHA-256: `…`
- `python3 scripts/check_repo.py` passes
- Tested: web (Chrome 130), Android 14, Windows 11 x64

## Known issues
- …

## Scope
No book content, DRM keys, or credentials are included.
```

## After release

- Update the “current version” table in `SECURITY.md` and its Chinese counterpart.
- Open a new `## [Unreleased]` section in `CHANGELOG.md`.
- If a binary changed, update `data/source-stats.json` in the release commit.
- Do not rewrite a published tag. If a release is broken, publish a new patch
  version and mark the old one as withdrawn in the notes.

## Hotfixes

A security fix may skip the normal minor cadence. It still needs:

- a private report or a clear issue,
- a minimal diff,
- a test or a documented manual verification,
- a release note that says what was fixed without exposing users.

## Rollback

Keep the previous release artifacts available. If a new build regresses, point
users to the previous tag and publish a patch that reverts the change. Never
silently replace an asset with a different binary.
