#!/usr/bin/env python3

# Copyright (c) ZeroC, Inc.

"""
process_linux_crash_dumps.py - generate crash reports from Linux core dumps

Uses ``coredumpctl`` to discover core dumps captured by systemd-coredump and
``gdb`` to extract backtraces and shared-library lists.

For every core dump found this script:
  1. Extracts the core file via ``coredumpctl dump``.
  2. Runs GDB in batch mode to produce a full backtrace (``thread apply all bt full``)
     and a shared-library listing (``info shared``).
  3. Copies the crashed executable and any workspace shared libraries into a
     dedicated report folder.

Example folder structure created:

    LinuxDumpReports/
      server-12345/
        backtrace.txt
        core
        server
        libIce.so.3.8

Arguments:
  --workspace   Root of your GitHub checkout (default: env GITHUB_WORKSPACE or cwd)
  --reports     Where to write the per-dump report folders (default: ./LinuxDumpReports)

Requires Python 3.13+ and the ``coredumpctl`` and ``gdb`` commands.
"""

import argparse
import json
import logging
import os
import re
import shutil
import subprocess
from pathlib import Path
from typing import Iterable, List

LOGGER = logging.getLogger(__name__)

# Matches lines from GDB's "info shared" output, e.g.:
#   0x00007f... 0x00007f... Yes         /home/runner/work/ice/ice/cpp/lib/libIce.so.3.8
#   0x00007f... 0x00007f... Yes (*)     /lib/x86_64-linux-gnu/libstdc++.so.6
SHARED_LIB_RE = re.compile(r"^\s*0x[0-9A-Fa-f]+\s+0x[0-9A-Fa-f]+\s+\S+(?:\s+\(\*\))?\s+(/.+)$")


def list_coredumps() -> list[dict]:
    """Return a list of core dump entries from ``coredumpctl``."""
    result = subprocess.run(
        ["coredumpctl", "list", "--json=short", "--no-pager"],
        capture_output=True,
        text=True,
    )
    if result.returncode != 0:
        LOGGER.info("coredumpctl list returned %d: %s", result.returncode, result.stderr.strip())
        return []
    try:
        return json.loads(result.stdout)
    except json.JSONDecodeError:
        LOGGER.warning("Failed to parse coredumpctl JSON output")
        return []


def extract_core(pid: int, dest: Path) -> bool:
    """Extract the core file for *pid* to *dest*. Returns True on success."""
    result = subprocess.run(
        ["coredumpctl", "dump", str(pid), "--output", str(dest), "--no-pager"],
        capture_output=True,
    )
    if result.returncode != 0:
        LOGGER.error("coredumpctl dump failed for PID %d: %s", pid, result.stderr.decode(errors="replace").strip())
        return False
    return True


def run_gdb(executable: Path, core: Path, output: Path) -> None:
    """Run GDB in batch mode and write backtrace + shared-library list to *output*."""
    cmd: List[str] = [
        "gdb",
        "-q",
        "-n",
        "-batch",
        "-ex",
        "thread apply all bt full",
        "-ex",
        "info shared",
        str(executable),
        "-c",
        str(core),
    ]
    LOGGER.debug("Running: %s", " ".join(cmd))
    with open(output, "w", encoding="utf-8", errors="replace") as fp:
        subprocess.run(cmd, stdout=fp, stderr=subprocess.STDOUT)


def parse_shared_libs(gdb_output: Path) -> Iterable[Path]:
    """Yield absolute *Path* objects for shared libraries found in GDB output."""
    with gdb_output.open(encoding="utf-8", errors="ignore") as fp:
        for line in fp:
            m = SHARED_LIB_RE.match(line)
            if m:
                yield Path(m.group(1).strip())


def inside_workspace(path: Path, workspace: Path) -> bool:
    """Return *True* if *path* is located inside *workspace*."""
    try:
        return Path(os.path.commonpath([workspace.resolve(), path.resolve()])) == workspace.resolve()
    except ValueError:
        return False


def process_dump(entry: dict, workspace: Path, reports_root: Path) -> None:
    """Generate report folder for a single core dump entry."""
    pid = entry["pid"]
    exe = entry.get("exe", "unknown")
    exe_path = Path(exe)
    exe_name = exe_path.name if exe_path.name else "unknown"

    report_dir = reports_root / f"{exe_name}-{pid}"
    report_dir.mkdir(parents=True, exist_ok=True)

    # Extract core file
    core_path = report_dir / "core"
    if not extract_core(pid, core_path):
        return

    # Run GDB to generate backtrace and shared library list
    backtrace_path = report_dir / "backtrace.txt"
    if exe_path.exists():
        run_gdb(exe_path, core_path, backtrace_path)
    else:
        LOGGER.warning("Executable not found: %s", exe_path)
        return

    # Copy the crashed executable
    if inside_workspace(exe_path, workspace):
        shutil.copy2(exe_path, report_dir / exe_name)
        LOGGER.info("Copied %s", exe_path)

    # Collect workspace shared libraries referenced by the core dump
    if backtrace_path.exists():
        for lib_path in parse_shared_libs(backtrace_path):
            if inside_workspace(lib_path, workspace) and lib_path.exists():
                shutil.copy2(lib_path, report_dir / lib_path.name)
                LOGGER.info("Copied %s", lib_path)
            else:
                LOGGER.debug("Skipping non-workspace library: %s", lib_path)


# ---------------------------------------------------------------------------
# CLI
# ---------------------------------------------------------------------------


def build_arg_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description="Generate crash reports from Linux core dumps")

    parser.add_argument(
        "--workspace",
        type=Path,
        default=Path(os.getenv("GITHUB_WORKSPACE", ".")).resolve(),
        help="GitHub workspace root",
    )

    parser.add_argument("--reports", type=Path, default=Path("LinuxDumpReports"), help="Output folder for reports")

    parser.add_argument("-v", "--verbose", action="count", default=0, help="Increase logging verbosity (-v or -vv)")
    return parser


def configure_logging(verbosity: int) -> None:
    level = logging.WARNING - (10 * min(verbosity, 2))  # WARNING→INFO→DEBUG
    logging.basicConfig(format="%(levelname)s: %(message)s", level=level)


def main(argv: List[str] | None = None) -> None:
    args = build_arg_parser().parse_args(argv)
    configure_logging(args.verbose)

    LOGGER.info("Workspace root: %s", args.workspace)

    entries = list_coredumps()
    if not entries:
        LOGGER.warning("No core dumps found")
        return

    for entry in entries:
        exe = entry.get("exe", "unknown")
        pid = entry["pid"]
        LOGGER.info("Processing core dump for %s (PID %d)", exe, pid)
        try:
            process_dump(entry, args.workspace, args.reports)
        except Exception as exc:
            LOGGER.error("Failed to process core dump for PID %d: %s", pid, exc)


if __name__ == "__main__":
    main()
