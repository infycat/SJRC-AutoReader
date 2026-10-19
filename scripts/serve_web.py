#!/usr/bin/env python3
"""Serve the single-file web app on localhost for Google sign-in.

The page needs an http:// origin; file:// is not sufficient because Google
Identity Services rejects a null origin.

Usage:
    python3 scripts/serve_web.py [--port 8080] [--no-browser]
"""

from __future__ import annotations

import argparse
import http.server
import socketserver
import webbrowser
from pathlib import Path

SCRIPT_DIR = Path(__file__).resolve().parent
# In a downloaded Web bundle, the HTML and this server sit side by side.
# In the source repository, the server lives under scripts/ and the HTML is in web/.
WEB = SCRIPT_DIR if (SCRIPT_DIR / "SJRC_AutoReader.html").is_file() else SCRIPT_DIR.parent / "web"


class Handler(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=str(WEB), **kwargs)

    def _rewrite_root(self) -> None:
        if self.path.split("?", 1)[0] in ("/", "/index.html"):
            self.path = "/SJRC_AutoReader.html"

    def do_GET(self):  # noqa: N802
        self._rewrite_root()
        return super().do_GET()

    def do_HEAD(self):  # noqa: N802
        self._rewrite_root()
        return super().do_HEAD()

    def end_headers(self):
        self.send_header("Cache-Control", "no-store, max-age=0")
        self.send_header("X-Content-Type-Options", "nosniff")
        self.send_header("Referrer-Policy", "no-referrer")
        super().end_headers()

    def log_message(self, fmt, *args):  # noqa: A003
        print(f"[web] {self.address_string()} {fmt % args}")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--port", type=int, default=8080)
    parser.add_argument("--bind", default="127.0.0.1")
    parser.add_argument("--no-browser", action="store_true")
    args = parser.parse_args()
    url = f"http://localhost:{args.port}/SJRC_AutoReader.html"
    socketserver.TCPServer.allow_reuse_address = True
    with socketserver.TCPServer((args.bind, args.port), Handler) as httpd:
        print(f"Serving {WEB}")
        print(f"Open: {url}")
        print("Google Cloud Console → OAuth client → Authorized JavaScript origins:")
        print(f"  http://localhost:{args.port}")
        print("Ctrl+C to stop.")
        if not args.no_browser:
            try:
                webbrowser.open(url)
            except Exception:  # noqa: BLE001
                pass
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            print("\nStopped.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
