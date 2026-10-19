#!/usr/bin/env python3
"""Create a deterministic source archive of the repository.

The archive contains one top-level directory (`SJRC-AutoReader-Repo/`), sorted
entries, a fixed timestamp, and stable permissions. Re-running on the same tree
produces the same SHA-256.

Usage:
    python3 scripts/package_release.py --version 1.0.0
    python3 scripts/package_release.py --output ../SJRC-AutoReader-Repo.zip
"""

from __future__ import annotations

import argparse
import hashlib
import os
import stat
import sys
import zipfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
FIXED_TIME = (2026, 10, 19, 0, 0, 0)
EXCLUDE_DIRS = {".git", ".github/workflows/.cache", "dist", "__pycache__", ".pytest_cache"}
EXCLUDE_SUFFIXES = {".pyc", ".pyo", ".zip"}
EXCLUDE_NAMES = {".DS_Store", "Thumbs.db"}


def include(path: Path) -> bool:
    parts = path.relative_to(ROOT).parts
    if any(part in EXCLUDE_DIRS for part in parts):
        return False
    if path.name in EXCLUDE_NAMES or path.suffix.lower() in EXCLUDE_SUFFIXES:
        return False
    if path.is_symlink():
        return False
    return path.is_file()


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def build(output: Path, version: str) -> tuple[int, int, str]:
    output = output.resolve()
    output.parent.mkdir(parents=True, exist_ok=True)
    if output.exists():
        output.unlink()

    files = sorted((p for p in ROOT.rglob("*") if include(p)), key=lambda p: p.as_posix())
    total = 0
    with zipfile.ZipFile(output, "w", compression=zipfile.ZIP_DEFLATED, compresslevel=9) as archive:
        for path in files:
            rel = path.relative_to(ROOT).as_posix()
            mode = stat.S_IMODE(path.stat().st_mode)
            info = zipfile.ZipInfo(f"SJRC-AutoReader-Repo/{rel}", date_time=FIXED_TIME)
            info.compress_type = zipfile.ZIP_DEFLATED
            info.create_system = 3
            info.external_attr = ((stat.S_IFREG | (0o755 if mode & 0o111 else 0o644)) << 16)
            archive.writestr(info, path.read_bytes(), compresslevel=9)
            total += path.stat().st_size
    digest = sha256(output)
    sidecar = output.with_name(output.name + ".sha256")
    sidecar.write_text(f"{digest}  {output.name}\n", encoding="utf-8")
    print(f"archive : {output}")
    print(f"version : {version}")
    print(f"files   : {len(files):,}")
    print(f"source  : {total:,} bytes")
    print(f"archive : {output.stat().st_size:,} bytes")
    print(f"sha256  : {digest}")
    print(f"sidecar : {sidecar}")
    return len(files), output.stat().st_size, digest


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--version", default="1.0.0", help="version used in the default name")
    parser.add_argument("--output", default=None, help="output zip path")
    args = parser.parse_args()
    output = Path(args.output) if args.output else ROOT / "dist" / f"SJRC-AutoReader-Repo-{args.version}.zip"
    if not output.is_absolute():
        output = (Path.cwd() / output).resolve()
    if output == ROOT or output.is_dir():
        print("error: output must be a file path", file=sys.stderr)
        return 2
    try:
        output.relative_to(ROOT)
    except ValueError:
        pass
    if output.exists() and output.resolve().parent == ROOT:
        print("error: refusing to overwrite a repository file", file=sys.stderr)
        return 2
    build(output, args.version)
    return 0


if __name__ == "__main__":
    sys.exit(main())
