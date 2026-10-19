# Android source

Built without an Android SDK. See [building.md](../../docs/en/building.md) for the
full rebuild options and [reverse-engineering.md](../../docs/en/reverse-engineering.md)
for how the wrapper was produced.

| File | Note |
|---|---|
| `AndroidManifest.xml` | Decoded manifest: package `club.sjrc.autoread`, launcher activity, `minSdk 23` |
| `index.html` | The embedded page (email/password sign-in only) |
| `MainActivity` (excerpt) | The WebView shell; the full class lives in `classes.dex` inside the APK |
| `BodyBuilder.smali` … `UseTimeEntity.smali` | 6 key excerpts from the official SJRC client, kept as protocol evidence |

## MainActivity (excerpt)

The checked-in APK contains six wrapper classes (`MainActivity`, `OAuthClient`,
`PopupChrome`, `Poster`, `Probe`, `WebServer`). Only the manifest and this excerpt
are kept in the source tree; the complete class can be disassembled from
`classes.dex` with any DEX tool.

```smali
.class public Lclub/sjrc/auto/MainActivity;
.super Landroid/app/Activity;

.method protected onCreate(Landroid/os/Bundle;)V
    .registers 6
    new-instance v0, Landroid/webkit/WebView;
    ...
    invoke-virtual {v1, v2}, Landroid/webkit/WebSettings;->setAllowFileAccessFromFileURLs(Z)V
    invoke-virtual {v1, v2}, Landroid/webkit/WebSettings;->setAllowUniversalAccessFromFileURLs(Z)V
    const-string v1, "file:///android_asset/index.html"
    invoke-virtual {v0, v1}, Landroid/webkit/WebView;->loadUrl(Ljava/lang/String;)V
```

Two things that are easy to get wrong:

1. **`android:name` must be the fully-qualified class.**
   `.MainActivity` expands to `<package>.MainActivity`, which is not where the class lives.

2. **File-URL access flags.**
   Without `setAllowUniversalAccessFromFileURLs(true)` a `file://` page
   cannot `fetch()` `https://api.sjrc.club`.

## Lifecycle

```text
onCreate   WebView + settings + loadUrl
onStop     loadUrl("javascript:…__appPause()")   → stops loop, PATCH /end
onDestroy  webView.destroy()
```

## Register trap

`.registers N` is the **total** register count; parameters occupy the last ones.
For `onCreate(Bundle)` (2 params) `.registers 2` leaves zero locals — `v0` *is* `this`,
and `const/4 v0, 0x0` nulls it out. Always leave room for locals.
