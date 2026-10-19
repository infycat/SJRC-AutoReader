#!/usr/bin/env python3
"""Build complete, deterministic platform download bundles for GitHub Releases."""

from __future__ import annotations

import argparse
import hashlib
import stat
import sys
import zipfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
FIXED_TIME = (2026, 10, 19, 0, 0, 0)

LICENSE = ROOT / "LICENSE"

PACKAGES = {
    "Web": {
        "name": "SJRC-AutoReader-Web.zip",
        "files": [
            (ROOT / "web" / "SJRC_AutoReader.html", "SJRC_AutoReader.html"),
            (ROOT / "web" / "Start-SJRC-AutoReader.bat", "Start-SJRC-AutoReader.bat"),
            (ROOT / "scripts" / "serve_web.py", "serve_web.py"),
            (LICENSE, "LICENSE"),
        ],
        "readme": """SJRC AutoReader 網頁版下載套件 / Web bundle

繁體中文
1. 將 ZIP 解壓縮到一般資料夾。
2. Windows：雙擊 Start-SJRC-AutoReader.bat。需要已安裝 Python 3；伺服器只綁定本機。
3. 瀏覽器會開啟 http://localhost:8080/SJRC_AutoReader.html。
4. Google 登入請在應用程式填入自己的 Google Client ID，並確保 OAuth 網頁來源包含 http://localhost:8080。
5. 關閉啟動器視窗或按 Ctrl+C 停止伺服器。

其他系統：在解壓縮目錄執行 python3 serve_web.py，再開啟上述網址。
不要直接以 file:// 開啟 HTML，Google OAuth 不接受 null origin。

English
1. Extract this ZIP to a regular folder.
2. On Windows, double-click Start-SJRC-AutoReader.bat. Python 3 must be installed; the server binds to localhost only.
3. Your browser opens http://localhost:8080/SJRC_AutoReader.html.
4. For Google sign-in, enter your own Google Client ID and authorize http://localhost:8080 as a JavaScript origin.
5. Close the launcher window or press Ctrl+C to stop the server.

Other systems: run python3 serve_web.py in the extracted folder, then open the URL above.
Do not open the HTML with file://; Google OAuth rejects a null origin.

研究用途；自動累積閱讀時數可能違反平台或學校規定。詳見 LICENSE 與專案法律說明。
For research use. Automated reading time may violate platform or school rules. See LICENSE and the project legal notice.
""",
    },
    "Android": {
        "name": "SJRC-AutoReader-Android.zip",
        "files": [
            (ROOT / "phone" / "SJRC-AutoRead.apk", "SJRC-AutoRead.apk"),
            (LICENSE, "LICENSE"),
        ],
        "readme": """SJRC AutoReader Android 下載套件 / Android bundle

繁體中文
1. 將 ZIP 解壓縮。
2. 開啟 SJRC-AutoRead.apk；若系統詢問，允許目前的檔案管理員安裝未知應用程式。
3. 安裝並開啟 SJRC AutoRead，以電郵、密碼及驗證碼登入。
4. 本套件是獨立外殼，不是官方 SJRC App 的更新；簽章不同。

English
1. Extract this ZIP.
2. Open SJRC-AutoRead.apk. If prompted, allow the file manager to install unknown apps.
3. Install and open SJRC AutoRead; sign in with email, password, and verification code.
4. This is a separate wrapper, not an update to the official SJRC app; the signing keys differ.

研究用途；自動累積閱讀時數可能違反平台或學校規定。詳見 LICENSE 與專案法律說明。
For research use. Automated reading time may violate platform or school rules. See LICENSE and the project legal notice.
""",
    },
    "Windows": {
        "name": "SJRC-AutoReader-Windows.zip",
        "files": [
            (ROOT / "pc" / "SJRC_AutoReader.exe", "SJRC_AutoReader.exe"),
            (LICENSE, "LICENSE"),
        ],
        "readme": """SJRC AutoReader Windows 下載套件 / Windows bundle

繁體中文
1. 將 ZIP 解壓縮到一般資料夾。
2. 執行 SJRC_AutoReader.exe。若 SmartScreen 顯示警告，先用隨附的 SJRC-AutoReader-Windows.zip.sha256 sidecar 驗證下載套件。
3. 使用本機 Google 登入頁，或貼上 access token。
4. 設定保存在目前 Windows 使用者的登錄檔中；移除方式請見專案安裝文件。

English
1. Extract this ZIP to a regular folder.
2. Run SJRC_AutoReader.exe. If SmartScreen warns, verify the bundle using SJRC-AutoReader-Windows.zip.sha256 first.
3. Use the local Google sign-in helper or paste an access token.
4. Settings are stored in the current user's registry; see the installation guide for removal.

研究用途；自動累積閱讀時數可能違反平台或學校規定。詳見 LICENSE 與專案法律說明。
For research use. Automated reading time may violate platform or school rules. See LICENSE and the project legal notice.
""",
    },
}


def sha256(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def zip_info(name: str) -> zipfile.ZipInfo:
    info = zipfile.ZipInfo(name, date_time=FIXED_TIME)
    info.compress_type = zipfile.ZIP_DEFLATED
    info.create_system = 3
    info.external_attr = ((stat.S_IFREG | 0o644) << 16)
    return info


def build(output: Path, files: list[tuple[Path, str]], readme: str) -> tuple[int, str]:
    payloads: list[tuple[str, bytes]] = []
    for source, name in files:
        if not source.is_file():
            raise FileNotFoundError(f"required package file missing: {source}")
        payloads.append((name, source.read_bytes()))
    payloads.append(("README.txt", readme.replace("\r\n", "\n").encode("utf-8")))

    checksums = "".join(f"{sha256(data)}  {name}\n" for name, data in sorted(payloads))
    payloads.append(("SHA256SUMS", checksums.encode("ascii")))

    output.parent.mkdir(parents=True, exist_ok=True)
    if output.exists():
        output.unlink()
    with zipfile.ZipFile(output, "w", compression=zipfile.ZIP_DEFLATED, compresslevel=9) as archive:
        for name, data in sorted(payloads):
            archive.writestr(zip_info(name), data, compresslevel=9)

    digest = sha256(output.read_bytes())
    sidecar = output.with_name(output.name + ".sha256")
    sidecar.write_text(f"{digest}  {output.name}\n", encoding="ascii")
    return output.stat().st_size, digest


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output-dir", default=str(ROOT / "dist"))
    args = parser.parse_args()
    output_dir = Path(args.output_dir)
    if not output_dir.is_absolute():
        output_dir = (Path.cwd() / output_dir).resolve()

    try:
        for platform, package in PACKAGES.items():
            output = output_dir / package["name"]
            size, digest = build(output, package["files"], package["readme"])
            print(f"{platform:7} {output.name} {size:,} bytes sha256:{digest}")
    except (OSError, zipfile.BadZipFile) as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
