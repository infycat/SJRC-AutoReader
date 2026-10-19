# Governance

[繁體中文](GOVERNANCE.zh.md) · **English**

## Model

SJRC AutoReader uses a **single-maintainer, lazy-consensus** model. The current
maintainer is listed in [AUTHORS.md](AUTHORS.md). Contributors propose changes
through issues and pull requests; the maintainer makes the final call when
consensus is not reached.

This is a small interoperability project, not a standards body. The goal of
governance is to keep decisions predictable and to protect users from unsafe
changes, not to build a hierarchy.

## Decision rules

- **Routine changes** (docs, typo fixes, tests, refactors with no behaviour
  change): merge after review when CI passes.
- **Behaviour changes** (heartbeat timing, retry policy, new endpoints, UI
  defaults): require an issue that explains the user-visible effect and the
  evidence for it, plus one maintainer approval.
- **Security-sensitive changes** (credential storage, network origin, build
  pipeline, anything that could leak a token): require a private discussion when
  appropriate and explicit maintainer approval.
- **Scope changes** (new platform, new dependency, bundling third-party code):
  require a written proposal with licence and maintenance analysis.

Consensus is assumed when no maintainer objects within 7 calendar days on a
non-urgent proposal. Objections must include a technical reason and a path
forward. Rejected proposals can be reopened with new evidence.

## Roles

| Role | Responsibilities |
|---|---|
| Maintainer | Final review, releases, security response, repository settings |
| Contributor | Issues, docs, tests, code, translations, reproductions |
| Reporter | Security reports, bug reports with logs that contain no secrets |

Contributors who land several high-quality changes and help with reviews may be
invited to become maintainers. There is no fixed quota.

## Releases

Releases follow [docs/en/release.md](docs/en/release.md). A release is a tagged
commit with a matching `CHANGELOG.md` entry and a source archive produced by
`scripts/package_release.py`. Binaries are attached as release assets, not
committed again to a second branch.

## Changes to this document

Governance changes use the same pull-request process. The maintainer must leave
the proposal open for at least 7 days unless the change is a clarification that
does not alter decision rights.
