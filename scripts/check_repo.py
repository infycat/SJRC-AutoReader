#!/usr/bin/env python3
"""Offline repository checks for SJRC-AutoReader-Repo.

No network access. Verifies required files, relative links and anchors, English /
Chinese documentation parity, JSON/CSV/XML validity, checksums, data statistics,
the inline web script, and accidental secrets.

Usage:
    python3 scripts/check_repo.py [--quiet] [--no-node]
"""

from __future__ import annotations

import argparse
import csv
import hashlib
import json
import os
import re
import shutil
import subprocess
import sys
import tempfile
import xml.etree.ElementTree as ET
from pathlib import Path
from urllib.parse import unquote, urlsplit

ROOT = Path(__file__).resolve().parents[1]

DOC_PAGES = [
    "getting-started", "installation", "configuration", "usage", "architecture",
    "api", "code-map", "reverse-engineering", "building", "testing", "release",
    "security", "privacy", "legal", "troubleshooting", "faq", "glossary",
]

REQUIRED = [
    "README.md", "README.zh.md", "LICENSE", "CHANGELOG.md", "CONTRIBUTING.md",
    "CONTRIBUTING.zh.md", "SECURITY.md", "SECURITY.zh.md", "CODE_OF_CONDUCT.md",
    "CODE_OF_CONDUCT.zh.md", "GOVERNANCE.md", "GOVERNANCE.zh.md", "SUPPORT.md",
    "SUPPORT.zh.md", "ROADMAP.md", "ROADMAP.zh.md", "AUTHORS.md", "NOTICE.md",
    "CITATION.cff", "CODE-OVERVIEW.md", "SHA256SUMS", "Makefile",
    ".editorconfig", ".gitattributes", ".gitignore",
    "scripts/check_repo.py", "scripts/package_release.py", "scripts/serve_web.py",
    "data/README.md", "data/api-endpoints.csv", "data/api-endpoints.json",
    "data/api-endpoints-by-module.md", "data/source-stats.json",
    "docs/README.md", "docs/assets/banner.svg", "docs/assets/logo.svg",
    "docs/assets/architecture.svg", "docs/assets/heartbeat-flow.svg",
    "docs/assets/data-flow.svg",
    ".github/workflows/ci.yml", ".github/workflows/release.yml",
    ".github/dependabot.yml", ".github/PULL_REQUEST_TEMPLATE.md",
    ".github/ISSUE_TEMPLATE/bug_report.yml",
    ".github/ISSUE_TEMPLATE/feature_request.yml",
    ".github/ISSUE_TEMPLATE/config.yml",
    "web/SJRC_AutoReader.html", "phone/SJRC-AutoRead.apk",
    "pc/SJRC_AutoReader.exe", "pc/src/main.cpp", "pc/src/build.bat",
    "sjrc-source/AndroidManifest.xml", "sjrc-source/net_endpoints.json",
]

# Binary or vendored trees that are not useful for link/secret scanning.
SKIP_LINK_DIRS = {"smali", ".git", "dist", "__pycache__"}
SECRET_SKIP_DIRS = {".git", "dist", "__pycache__"}
TEXT_SUFFIXES = {
    ".md", ".py", ".yml", ".yaml", ".json", ".csv", ".html", ".txt",
    ".cpp", ".h", ".bat", ".cff", ".rc", ".xml", ".editorconfig", ".smali",
}
TEXT_NAMES = {".gitignore", ".gitattributes", "Makefile", "LICENSE"}

errors: list[str] = []
warnings: list[str] = []
checks = 0


def ok() -> None:
    global checks
    checks += 1


def fail(message: str) -> None:
    errors.append(message)


def warn(message: str) -> None:
    warnings.append(message)


def rel(path: Path) -> str:
    try:
        return str(path.relative_to(ROOT))
    except ValueError:
        return str(path)


def read_text(path: Path) -> str:
    return path.read_text(encoding="utf-8", errors="replace")


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


# --------------------------------------------------------------------------
# Structure
# --------------------------------------------------------------------------

def check_required() -> None:
    for name in REQUIRED:
        path = ROOT / name
        if path.exists() and path.stat().st_size > 0:
            ok()
        else:
            fail(f"required file missing or empty: {name}")

    for lang in ("en", "zh"):
        for page in DOC_PAGES:
            path = ROOT / "docs" / lang / f"{page}.md"
            if path.is_file() and path.stat().st_size > 400:
                ok()
            else:
                fail(f"documentation page missing or too short: {rel(path)}")

    for lang in ("en", "zh"):
        found = {p.stem for p in (ROOT / "docs" / lang).glob("*.md")}
        expected = set(DOC_PAGES)
        if found != expected:
            fail(f"docs/{lang} pages differ from expected: "
                 f"missing={sorted(expected - found)} extra={sorted(found - expected)}")
        else:
            ok()


def check_placeholders() -> None:
    offenders = []
    for path in ROOT.rglob("*"):
        if not path.is_file() or any(part in SKIP_LINK_DIRS for part in path.parts):
            continue
        if path.name.startswith(".old-") or path.suffix.lower() in {".apk", ".exe", ".png", ".ico", ".zip"}:
            continue
        if path.suffix.lower() not in TEXT_SUFFIXES and path.name not in TEXT_NAMES:
            continue
        text = read_text(path)
        markers = (
            "github.com/" + "OWNER/",
            "repo%3A" + "OWNER%2F",
            "<" + "OWNER>",
            "YOUR" + "_NAME",
        )
        for marker in markers:
            if marker in text:
                offenders.append(f"{rel(path)} contains placeholder {marker!r}")
    if offenders:
        errors.extend(offenders)
    else:
        ok()


# --------------------------------------------------------------------------
# Markdown links and anchors
# --------------------------------------------------------------------------

FENCE_RE = re.compile(r"^```.*?^```", re.MULTILINE | re.DOTALL)
INLINE_CODE_RE = re.compile(r"`[^`\n]*`")
COMMENT_RE = re.compile(r"<!--.*?-->", re.DOTALL)
LINK_RE = re.compile(r"\[[^\]]*\]\(([^)\s]+)(?:\s+['\"][^'\"]*['\"])?\)")
HTML_LINK_RE = re.compile(r"<(?:img|a)\b[^>]*\b(?:src|href)=[\"']([^\"']+)[\"']", re.IGNORECASE)
HEADING_RE = re.compile(r"^(#{1,6})\s+(.+?)\s*$", re.MULTILINE)


def slugify(heading: str) -> str:
    text = heading.strip()
    text = re.sub(r"<[^>]+>", "", text)
    text = re.sub(r"!\[([^\]]*)\]\([^)]*\)", r"\1", text)
    text = re.sub(r"\[([^\]]+)\]\([^)]*\)", r"\1", text)
    text = re.sub(r"[`*_~]", "", text)
    text = text.lower()
    text = re.sub(r"[^\w\s\-]", "", text, flags=re.UNICODE)
    text = re.sub(r"\s+", "-", text)
    return text


def anchors_for(path: Path) -> set[str]:
    text = FENCE_RE.sub("", read_text(path))
    seen: dict[str, int] = {}
    anchors: set[str] = set()
    for match in HEADING_RE.finditer(text):
        base = slugify(match.group(2))
        count = seen.get(base, 0)
        anchors.add(base if count == 0 else f"{base}-{count}")
        seen[base] = count + 1
    return anchors


def check_links() -> None:
    cache: dict[Path, set[str]] = {}
    total = 0
    broken: list[str] = []

    for path in ROOT.rglob("*.md"):
        if any(part in SKIP_LINK_DIRS for part in path.parts):
            continue
        text = read_text(path)
        text = COMMENT_RE.sub("", text)
        text = FENCE_RE.sub("", text)
        text = INLINE_CODE_RE.sub("", text)
        targets = LINK_RE.findall(text) + HTML_LINK_RE.findall(text)
        for raw in targets:
            target = raw.strip()
            if not target or target.startswith(("http://", "https://", "mailto:", "tel:", "data:", "javascript:")):
                continue
            total += 1
            parsed = urlsplit(target)
            link_path = unquote(parsed.path)
            fragment = unquote(parsed.fragment)
            if link_path:
                candidate = (path.parent / link_path).resolve()
                if ROOT not in candidate.parents and candidate != ROOT:
                    broken.append(f"{rel(path)} -> {target} escapes repository")
                    continue
                if not candidate.exists():
                    broken.append(f"{rel(path)} -> {target} (missing)")
                    continue
                if fragment and candidate.is_file() and candidate.suffix.lower() == ".md":
                    anchors = cache.setdefault(candidate, anchors_for(candidate))
                    if fragment not in anchors:
                        broken.append(f"{rel(path)} -> {target} (missing anchor #{fragment})")
                        continue
            elif fragment:
                anchors = cache.setdefault(path, anchors_for(path))
                if fragment not in anchors:
                    broken.append(f"{rel(path)} -> {target} (missing anchor #{fragment})")
                    continue
            ok()

    if broken:
        errors.extend(broken[:40])
        if len(broken) > 40:
            errors.append(f"... and {len(broken) - 40} more broken links")
    else:
        print(f"links: {total} relative links and anchors OK")


def check_parity() -> None:
    for page in DOC_PAGES:
        en = ROOT / "docs" / "en" / f"{page}.md"
        zh = ROOT / "docs" / "zh" / f"{page}.md"
        if not en.exists() or not zh.exists():
            continue
        def levels(path: Path) -> list[int]:
            text = FENCE_RE.sub("", read_text(path))
            return [len(m.group(1)) for m in HEADING_RE.finditer(text)]
        a, b = levels(en), levels(zh)
        if a != b:
            fail(f"heading structure differs: docs/en/{page}.md {a} vs docs/zh/{page}.md {b}")
        else:
            ok()
        if f"../zh/{page}.md" not in read_text(en):
            fail(f"docs/en/{page}.md does not link to its Chinese counterpart")
        if f"../en/{page}.md" not in read_text(zh):
            fail(f"docs/zh/{page}.md does not link to its English counterpart")


# --------------------------------------------------------------------------
# Data, images and checksums
# --------------------------------------------------------------------------

def check_data() -> None:
    try:
        methods = json.loads(read_text(ROOT / "data/api-endpoints.json"))
        assert methods["method_count"] == len(methods["methods"])
        assert methods["method_count"] == 98
        assert methods["unique_endpoint_count"] == 67
        ok()
    except Exception as exc:  # noqa: BLE001
        fail(f"data/api-endpoints.json invalid: {exc}")

    try:
        with (ROOT / "data/api-endpoints.csv").open(newline="", encoding="utf-8") as handle:
            rows = list(csv.DictReader(handle))
        assert len(rows) == 98, len(rows)
        assert set(rows[0]) == {"http", "path", "method", "class", "description"}, set(rows[0])
        ok()
    except Exception as exc:  # noqa: BLE001
        fail(f"data/api-endpoints.csv invalid: {exc}")

    try:
        stats = json.loads(read_text(ROOT / "data/source-stats.json"))
        files = len(list((ROOT / "sjrc-source" / "smali").rglob("*.smali")))
        assert stats["smali"]["files"] == files == 4888, (stats["smali"]["files"], files)
        for name, info in stats["release_artifacts"].items():
            target = ROOT / name
            assert target.stat().st_size == info["bytes"], name
            assert sha256(target) == info["sha256"], name
        ok()
    except Exception as exc:  # noqa: BLE001
        fail(f"data/source-stats.json invalid or stale: {exc}")

    try:
        with (ROOT / "SHA256SUMS").open(encoding="utf-8") as handle:
            lines = [line.split(None, 1) for line in handle if line.strip()]
        assert lines, "empty SHA256SUMS"
        for digest, name in lines:
            name = name.strip().lstrip("*")
            target = ROOT / name
            assert target.is_file(), f"missing {name}"
            assert sha256(target) == digest, f"hash mismatch for {name}"
        ok()
    except Exception as exc:  # noqa: BLE001
        fail(f"SHA256SUMS invalid: {exc}")

    for path in ROOT.rglob("*.json"):
        if any(part in SKIP_LINK_DIRS for part in path.parts):
            continue
        try:
            json.loads(read_text(path))
            ok()
        except Exception as exc:  # noqa: BLE001
            fail(f"invalid JSON {rel(path)}: {exc}")

    for path in ROOT.rglob("*.svg"):
        try:
            ET.parse(path)
            ok()
        except Exception as exc:  # noqa: BLE001
            fail(f"invalid SVG {rel(path)}: {exc}")

    try:
        ET.parse(ROOT / "phone" / "src" / "AndroidManifest.xml")
        # sjrc-source/AndroidManifest.xml is a decoded attribute dump, not XML.
        assert (ROOT / "sjrc-source" / "AndroidManifest.xml").stat().st_size > 40000
        ok()
    except Exception as exc:  # noqa: BLE001
        fail(f"invalid AndroidManifest.xml: {exc}")


def check_web_script(no_node: bool) -> None:
    html = read_text(ROOT / "web" / "SJRC_AutoReader.html")
    scripts = re.findall(r"<script(?:\s[^>]*)?>(.*?)</script>", html, re.DOTALL | re.IGNORECASE)
    if not scripts:
        fail("web/SJRC_AutoReader.html contains no inline <script>")
        return
    inline = scripts[-1]
    if len(inline) < 10000:
        fail("web inline script looks unexpectedly small")
        return
    ok()
    node = shutil.which("node")
    if not node or no_node:
        warn("node not found or --no-node set; skipped inline JavaScript syntax check")
        return
    with tempfile.NamedTemporaryFile("w", suffix=".js", delete=False, encoding="utf-8") as handle:
        handle.write(inline)
        temp = handle.name
    try:
        result = subprocess.run([node, "--check", temp], capture_output=True, text=True)
        if result.returncode != 0:
            fail(f"web inline JavaScript syntax error: {result.stderr.strip()[:500]}")
        else:
            ok()
    finally:
        os.unlink(temp)


# --------------------------------------------------------------------------
# Secret scan
# --------------------------------------------------------------------------

SECRET_PATTERNS = [
    ("private key block", re.compile(r"-----BEGIN (?:RSA |EC |OPENSSH |DSA )?PRIVATE KEY-----")),
    ("base64-encoded private key", re.compile("LS0tLS1CRUdJTiBQUklWQVRFIEtFWS0t" + "LS0t")),
    ("GitHub token", re.compile(r"\bgh[pousr]_[A-Za-z0-9]{30,}\b")),
    ("GitHub fine-grained token", re.compile(r"\bgithub_pat_[A-Za-z0-9_]{40,}\b")),
    ("AWS access key", re.compile(r"\bAKIA[0-9A-Z]{16}\b")),
    ("Slack token", re.compile(r"\bxox[baprs]-[A-Za-z0-9-]{20,}\b")),
    ("JWT", re.compile(r"\beyJ[A-Za-z0-9_-]{16,}\.eyJ[A-Za-z0-9_-]{16,}\.[A-Za-z0-9_-]{16,}\b")),
]


def check_secrets() -> None:
    suspicious = []
    for path in ROOT.rglob("*"):
        if not path.is_file():
            continue
        if any(part in SECRET_SKIP_DIRS for part in path.parts):
            continue
        if path.suffix.lower() not in TEXT_SUFFIXES and path.name not in TEXT_NAMES:
            continue
        if path.stat().st_size > 2_000_000:
            continue
        text = read_text(path)
        for label, pattern in SECRET_PATTERNS:
            for match in pattern.finditer(text):
                suspicious.append(f"{rel(path)}: possible {label}: {match.group(0)[:12]}…")
    if suspicious:
        errors.extend(suspicious)
    else:
        ok()

    risky = [p for p in ROOT.rglob("*") if p.suffix.lower() in {".jks", ".keystore", ".pem", ".p12"}]
    if risky:
        errors.append(f"key material present: {[rel(p) for p in risky][:10]}")
    else:
        ok()


# --------------------------------------------------------------------------
# Redaction markers
# --------------------------------------------------------------------------

REDACTED_FILES = [
    "sjrc-source/smali/club/sjrc/app/utils/AES.smali",
    "sjrc-source/smali/club/sjrc/app/utils/AESDecrypt.smali",
    "sjrc-source/smali/club/sjrc/app/BuildConfig.smali",
    "sjrc-source/smali/club/sjrc/app/utils/RSAOAEPDecryptor.smali",
    "sjrc-source/smali/club/sjrc/app/utils/RSAOAEPDecryptor$privateKey$2.smali",
]


def check_redactions() -> None:
    for name in REDACTED_FILES:
        path = ROOT / name
        if not path.exists():
            fail(f"redacted file missing: {name}")
            continue
        if "SECURITY REDACTION" not in read_text(path):
            fail(f"redaction marker missing in {name}")
        else:
            ok()
    try:
        stats = json.loads(read_text(ROOT / "data" / "source-stats.json"))
        if not stats.get("redactions", {}).get("applied"):
            fail("data/source-stats.json does not record applied redactions")
        else:
            ok()
    except Exception as exc:  # noqa: BLE001
        fail(f"invalid source-stats.json: {exc}")


# --------------------------------------------------------------------------
# YAML (optional)
# --------------------------------------------------------------------------

def check_yaml() -> None:
    try:
        import yaml  # type: ignore
    except ImportError:
        warn("PyYAML not installed; skipped YAML syntax checks (pip install -r requirements-dev.txt)")
        return
    for path in (ROOT / ".github").rglob("*.yml"):
        try:
            yaml.safe_load(read_text(path))
            ok()
        except Exception as exc:  # noqa: BLE001
            fail(f"invalid YAML {rel(path)}: {exc}")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--quiet", action="store_true")
    parser.add_argument("--no-node", action="store_true")
    args = parser.parse_args()

    check_required()
    check_placeholders()
    check_links()
    check_parity()
    check_data()
    check_web_script(args.no_node)
    check_redactions()
    check_secrets()
    check_yaml()

    # Cross-check the two top-level indexes.
    docs_index = read_text(ROOT / "docs" / "README.md")
    for page in DOC_PAGES:
        if f"en/{page}.md" not in docs_index or f"zh/{page}.md" not in docs_index:
            fail(f"docs/README.md does not index {page}.md")
    if "17" not in docs_index:
        warn("docs/README.md does not mention the page count (17)")
    overview = read_text(ROOT / "CODE-OVERVIEW.md")
    if "17 篇" not in overview:
        fail("CODE-OVERVIEW.md does not state the current 17-page docs count")

    if warnings and not args.quiet:
        print("\nWarnings:")
        for message in warnings:
            print("  !", message)

    if errors:
        print("\nFAILED:")
        for message in errors:
            print("  x", message)
        print(f"\n{len(errors)} error(s), {len(warnings)} warning(s), {checks} checks passed")
        return 1

    print(f"\nOK: {checks} checks passed, {len(warnings)} warning(s)")
    print("Repository is ready to package.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
