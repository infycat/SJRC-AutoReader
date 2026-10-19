# Web source

The product **is** the source. There is no build step.

| File | Note |
|---|---|
| `../../web/SJRC_AutoReader.html` | Single-file app: HTML + CSS + vanilla JS, icon inlined as base64 |

## Structure

```text
<style>
  :root {           /* colors sampled from the original APK */
    --acc  #3dcab1  /* colors.xml → color_theme      */
    --hl   #c0ed72  /* colors.xml → highlight_green  */
    --dim  #5f716c  /* darkened to pass WCAG AA      */
  }
</style>

<script>
  api(path, opts)        fetch wrapper: headers, token, schema
  refreshAccessToken()   POST /token/refresh
  initGsi()              Google Identity Services setup
  preLogin()             google/pre-login → token or ticket
  onGoogleCredential()   GIS credential callback
  doLogin()              ticket + verification code → tokens
  heartbeat()            PATCH /reading-time  { sessionId, time: 3 }
  turnPage()             POST  /reading-pos
  updateProgress()       target progress
  tick()                 1s driver
</script>
```

## Run locally

Must be served over `http://` — `file://` has origin `null` and Google rejects it.

```bash
cd web
python3 -m http.server 8080
```

## Differences from the APK asset

The APK embeds a variant (`phone/src/index.html`) that adds an email/password
form, removes the usable Google path (Google blocks OAuth in WebViews), and adds
`window.__appPause()` for the native `onStop` hook. The web page itself has no
email/password form; its only sign-in method is Google.
