# Legal and acceptable use

[中文](../zh/legal.md) · English

> This page is a project policy, not legal advice. Laws differ by country and by
> institution. If you are unsure, ask a qualified person before using the tool.

## Licence scope

Original code and documentation in this repository are released under the MIT
licence in [LICENSE](../../LICENSE). The MIT licence does **not** cover:

- The SJRC client, its decompiled classes under `sjrc-source/`, or the smali
  excerpts in `phone/src/`
- Book content, covers, metadata, or trademarks
- Third-party libraries and engines referenced by the client
- The names “SJRC”, “金閱閣”, or “校園版”

Those remain the property of their owners. See [NOTICE.md](../../NOTICE.md).

## No affiliation

This is an independent interoperability project. It is not affiliated with,
endorsed by, sponsored by, or supported by SJRC, 金閱閣, any school, or any
publisher. Do not present it as an official client.

## Terms of service and institutional rules

Automating reading time may violate:

- the platform's terms of service,
- your school's academic-integrity or library rules,
- a scholarship, visa, or programme condition that requires genuine reading.

The maintainers cannot tell you whether your use is permitted. You are
responsible for checking. A reading-time counter is not proof of learning, and
using a tool to inflate it may have consequences beyond this project.

## Reverse engineering

The repository contains protocol observations and a small number of decompiled
excerpts. The purpose is interoperability: understanding the request format well
enough to write a client that behaves predictably. The legal status of
reverse engineering and of quoting decompiled code varies by jurisdiction. The
project does not claim that every use is permitted everywhere.

If you are a rights holder and believe a file exceeds commentary or
interoperability, use the takedown route in [NOTICE.md](../../NOTICE.md).

## No DRM circumvention

This project does not include or implement:

- AES keys, IVs, or per-book keys
- RSA private keys
- Download or decryption endpoints
- Book content
- Authentication or quota bypass

Documentation may describe that a protection mechanism exists, because that is
part of the client's architecture. It does not provide the means to bypass it.

## Acceptable use

You may:

- Study the protocol and the client structure
- Run the tool on your own account, where permitted
- Fork and modify the code under the MIT terms for original parts
- Write about the project and cite it

You may not:

- Use the project to access another person's account
- Redistribute book content or keys
- Present the project as official or endorsed
- Use it to defraud a school, employer, or programme
- Remove the licence or notices from original parts
- Claim the decompiled material is MIT-licensed

## No warranty

The software is provided “as is”, without warranty of any kind, express or
implied, including merchantability, fitness for a particular purpose, and
non-infringement. The authors are not liable for account suspension, lost data,
academic consequences, or any other damages arising from use. See
[LICENSE](../../LICENSE).

## Privacy and security

See [privacy.md](privacy.md) and [security.md](security.md). The project has no
backend and cannot recover an account or delete upstream data.

## Changes

This policy may change with the project. A change that expands data collection or
weakens the no-content rule requires a pull request and an explicit note in
`CHANGELOG.md`.
