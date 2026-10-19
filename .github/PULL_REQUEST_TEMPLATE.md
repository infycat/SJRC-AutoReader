## Summary

<!-- What changed, and why? One or two sentences. -->

## Scope

- [ ] Web (`web/SJRC_AutoReader.html`)
- [ ] Android (`phone/`)
- [ ] Windows (`pc/`)
- [ ] Documentation (`docs/`, root README)
- [ ] Data / tooling / CI
- [ ] Reference source only (`sjrc-source/`, no behavioural change)

## Protocol impact

<!-- Does this change headers, units, endpoints, session lifecycle, or auth? -->

- [ ] No protocol change
- [ ] Protocol change, with evidence: <!-- smali class or Swagger operation -->

## Verification

<!-- Commands actually run, and their result. Do not claim checks you did not run. -->

```text
make check
# plus any platform-specific test
```

- [ ] `make check` passes
- [ ] Web inline script passes `node --check`
- [ ] English and Chinese docs remain structurally aligned
- [ ] No token, password, verification code, personal data, or key material added
- [ ] No new runtime dependency
- [ ] Binary changes are intentional and `SHA256SUMS` / `data/source-stats.json` are updated

## Screenshots or logs

<!-- Only if useful. Redact personal data. -->

## Checklist

- [ ] I read CONTRIBUTING.md and CODE_OF_CONDUCT.md
- [ ] I did not include generated build output or editor files
- [ ] I described any known limitation or follow-up
