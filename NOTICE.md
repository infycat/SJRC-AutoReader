# NOTICE

SJRC AutoReader is an independent interoperability project. It is **not**
affiliated with, endorsed by, or supported by SJRC, 金閱閣, any school, or any
publisher.

## Licence scope

The MIT licence in [LICENSE](LICENSE) covers the original code and documentation
created for this project. It does **not** cover:

- The SJRC Android application or the small `*.smali` excerpts under
  `phone/src/`
- Book content, cover images, metadata, or trademarks
- Third-party libraries, engines, or assets referenced or embedded in the
  original client
- The names "SJRC", "金閱閣", "校園版", or any associated logos

Those materials remain the property of their respective owners. The full
decompiled reference tree is not included in this repository. If you are a
rights holder and believe a file exceeds the project's interoperability purpose,
see the takedown process below.

## Upstream components observed in the client

The decompiled excerpts reference, among others:

- Android, Google Play services, Google Sign-In, Firebase
- Retrofit, OkHttp, Gson
- Readium, FBReader, skyepub, Adobe ADEPT
- PDFium, DjVuLibre, k2pdfopt, MuPDF-related components
- Bouncy Castle

Their licences are their own. This repository does not redistribute their full
source or binaries except where already present in the small wrapper APK and
Windows executable built for this project.

## Trademarks

All trademarks are used descriptively to identify the service being studied. No
claim of ownership is made.

## No content or keys

This repository does **not** include:

- Book files or decrypted content
- AES keys, IVs, RSA private keys, or per-book keys
- Access tokens, refresh tokens, passwords, or verification codes
- User account data

The full decompiled source tree is not distributed here. The repository includes
no AES keys, IVs, RSA private keys, access tokens, or book-specific keys.

Documentation may describe the *existence* and *shape* of a protection mechanism
for interoperability analysis, but not the secret material itself.

## Takedown and corrections

If you are a rights holder and want a file reviewed, open a minimal issue asking
for a private contact route, or use the repository's private security advisory
channel. Include the exact path, the right you believe is infringed, and your
relationship to the material. We will respond in good faith and remove or amend
material where appropriate. Do not include personal data in a public issue.

## Disclaimer

The software is provided "as is", without warranty of any kind. Automating
reading time may violate the platform's terms of service or your institution's
rules. You are responsible for how you use it. See [LICENSE](LICENSE).
