#!/usr/bin/env python3

# Copyright (c) ZeroC, Inc.

"""
process_crash_dumps.py - gather binaries & PDBs referenced by crash dumps

For every *.dmp* file found in the specified dumps directory this script:
  1. Launches `cdb.exe` to generate a *modules.txt* file containing full module list corresponding
     to the dlls and executables loaded by the crashed process.
  2. Parses the modules and filter outs any modules that are located outside the GitHub workspace,
     this is to avoid copying binaries not belonging to the current build.
  3. Copies each matching binary **and** its companion PDB (if found) to a dedicated sub-folder under
     specified reports directory.

Example folder structure created:

    Reports/
      server.exe.9568.dmp/
        modules.txt
        server.exe
        server.pdb
        ice38a0.dll
        ice38a0.pdb

Arguments:
  --dumps       Path containing *.dmp dump files
  --workspace   Root of your GitHub checkout (default: env GITHUB_WORKSPACE or cwd)
  --reports     Where to write the per-dump report folders (default: ./Reports)
  --cdb         Full path to cdb.exe if it isn't in PATH

Requires Python 3.13+.
"""

import argparse
import logging
import os
import re
import shutil
import subprocess
from pathlib import Path
from typing import Iterable, List

LOGGER = logging.getLogger(__name__)

IMAGE_PATH_RE = re.compile(r"^\s*Image\s+path:\s+(.+)$", re.IGNORECASE)


def run_cdb(dump: Path, modules_txt: Path, cdb_exe: str) -> None:
    """Run *cdb* on *dump* and write its verbose module list to *modules_txt*."""
    LOGGER.debug("Running cdb on %s", dump)
    cmd: List[str] = [
        cdb_exe,
        "-z",
        str(dump),
        "-c",
        ".symfix; .reload; lmv; q",
        "-logo",
        str(modules_txt),
    ]
    try:
        subprocess.run(cmd, check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    except FileNotFoundError as exc:
        LOGGER.error("Could not find cdb.exe - specify with --cdb or add it to PATH")
        raise SystemExit(1) from exc


def parse_image_paths(modules_txt: Path) -> Iterable[Path]:
    """Yield absolute *Path* objects for every `Image path:` found."""
    with modules_txt.open(encoding="utf-8", errors="ignore") as fp:
        for line in fp:
            m = IMAGE_PATH_RE.match(line)
            if m:
                raw = m.group(1).strip().strip("\r\n\"')(")  # clean quotes/newlines
                if raw:
                    yield Path(raw)


def inside_workspace(path: Path, workspace: Path) -> bool:
    """Return *True* if *path* is located inside *workspace* (case-insensitive)."""
    try:
        return Path(os.path.commonpath([workspace.resolve(), path.resolve()])) == workspace.resolve()
    except ValueError:
        # drives differ on Windows, cannot be inside
        return False


def copy_with_pdb(binary: Path, dest_dir: Path) -> None:
    """Copy *binary* and its sibling .pdb (if any) into *dest_dir*."""
    dest_dir.mkdir(parents=True, exist_ok=True)

    for src in (binary, binary.with_suffix(".pdb")):
        if src.exists():
            shutil.copy2(src, dest_dir / src.name)
            LOGGER.info("Copied %s → %s", src, dest_dir / src.name)
        else:
            LOGGER.debug("PDB not found: %s", src)


def process_dump(dump: Path, workspace: Path, reports_root: Path, cdb_exe: str) -> None:
    """Generate report folder for a single dump file."""
    report_dir = reports_root / dump.name
    report_dir.mkdir(parents=True, exist_ok=True)

    modules_txt = report_dir / "modules.txt"
    run_cdb(dump, modules_txt, cdb_exe)

    for img_path in parse_image_paths(modules_txt):
        if inside_workspace(img_path, workspace):
            copy_with_pdb(img_path, report_dir)
        else:
            LOGGER.debug("Skipping non-workspace module: %s", img_path)

    # Copy the dump file itself to the report directory
    shutil.copy2(dump, report_dir / dump.name)


# ---------------------------------------------------------------------------
# CLI
# ---------------------------------------------------------------------------

def build_arg_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description="Collect dump-related binaries and PDBs")
    parser.add_argument(
        "--dumps",
        type=Path,
        default=Path("dumps"),
        help="Folder containing .dmp Windows dump files")

    parser.add_argument(
        "--workspace",
        type=Path,
        default=Path(os.getenv("GITHUB_WORKSPACE", ".")).resolve(),
        help="GitHub workspace root")

    parser.add_argument(
        "--reports",
        type=Path,
        default=Path("Reports"),
        help="Output folder for reports")

    parser.add_argument(
        "--cdb",
        default="cdb",
        help="Path to cdb.exe (defaults to one on PATH)")

    parser.add_argument(
        "-v",
        "--verbose",
        action="count",
        default=0,
        help="Increase logging verbosity (-v or -vv)")
    return parser


def configure_logging(verbosity: int) -> None:
    level = logging.WARNING - (10 * min(verbosity, 2))  # WARNING→INFO→DEBUG
    logging.basicConfig(format="%(levelname)s: %(message)s", level=level)


def main(argv: List[str] | None = None) -> None:
    args = build_arg_parser().parse_args(argv)
    configure_logging(args.verbose)

    LOGGER.info("Workspace root: %s", args.workspace)

    dumps_found = list(args.dumps.glob("*.dmp"))
    if not dumps_found:
        LOGGER.warning("No .dmp files found in %s", args.dumps)
        return

    for dump in dumps_found:
        LOGGER.info("Processing %s", dump)
        try:
            process_dump(dump, args.workspace, args.reports, args.cdb)
        except subprocess.CalledProcessError as exc:
            LOGGER.error("cdb failed for %s (exit code %s)", dump, exc.returncode)


if __name__ == "__main__":
    main()
