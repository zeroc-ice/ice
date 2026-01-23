#!/usr/bin/env python3
"""
Index Ice C++ PDB files for source server debugging.

This script prepares Ice C++ builds for source server debugging by:

1. Scanning PDB files in cpp/bin/<platform>/<config>/ directories to discover referenced source files
2. Copying all unique source files to an output directory preserving their relative paths
3. Embedding srcsrv streams in each PDB that map source paths to HTTP URLs

The output directory structure is:

    <output>/<version>/sources/  - Copy of all source files referenced by PDBs
    <output>/<version>/pdbs/     - PDB files with embedded srcsrv streams

When a debugger loads these PDBs, it can automatically fetch source files from
the configured HTTP server (e.g., https://sources.zeroc.com/ice/<version>/<path>).

Requirements:

    - Python 3.12 or later
    - Windows SDK tools: srctool.exe (extracts source info from PDBs) and
      pdbstr.exe (writes srcsrv streams to PDBs)

Usage:

    python index-ice-pdb-sources.py -s <ice-source-dir> --version <version> \\
        --source-base-url <url> -o <output-dir>
"""

import argparse
import shutil
import subprocess
import sys
import tempfile
from collections.abc import Iterator
from dataclasses import dataclass, field
from datetime import datetime, timezone
from pathlib import Path

# Platform/configuration combinations to search for PDBs
PLATFORM_CONFIGS = ["x64/Debug", "x64/Release", "Win32/Debug", "Win32/Release"]

# C++ source file extensions to include
CPP_EXTENSIONS = frozenset({".cpp", ".h"})


@dataclass
class PdbSourceInfo:
    """Information about sources indexed in a PDB file."""

    pdb_path: Path
    source_files: list[Path] = field(default_factory=list)


def parse_args() -> argparse.Namespace:
    """Parse command line arguments."""
    parser = argparse.ArgumentParser(
        description="Index source files from PDB files in Ice C++ builds.",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  %(prog)s -s C:\\workspace\\ice --version 3.7.10 --source-base-url https://sources.zeroc.com/ice -o C:\\output\\symbols
  %(prog)s -s C:\\workspace\\ice --version 3.7.10-nightly.20251210.2 --source-base-url https://sources.zeroc.com/ice -o symbols --verbose
        """,
    )
    parser.add_argument(
        "--source-dir",
        "-s",
        type=Path,
        required=True,
        help="Path to the Ice source directory (e.g., C:\\workspace\\ice)",
    )
    parser.add_argument(
        "--version",
        required=True,
        help="Version string for the source URL (e.g., 3.7.10-nightly.20251210.2)",
    )
    parser.add_argument(
        "--source-base-url",
        required=True,
        help="Base URL for source files (e.g., https://sources.zeroc.com/ice)",
    )
    parser.add_argument(
        "--output-dir",
        "-o",
        type=Path,
        required=True,
        help="Output directory (creates '<version>/sources' and '<version>/pdbs' subdirectories)",
    )
    parser.add_argument(
        "--verbose",
        "-v",
        action="store_true",
        help="Enable verbose output",
    )
    return parser.parse_args()


def find_pdb_files(source_dir: Path) -> Iterator[Path]:
    """
    Find all PDB files in the Ice build directories.

    Searches in <source_dir>/cpp/bin/<platform>/<config>/ for each
    platform/configuration combination.

    Args:
        source_dir: Path to the Ice source directory.

    Yields:
        Paths to PDB files found.
    """
    bin_dir = source_dir / "cpp" / "bin"

    if not bin_dir.exists():
        raise FileNotFoundError(f"Bin directory not found: {bin_dir}")

    for platform_config in PLATFORM_CONFIGS:
        config_dir = bin_dir / platform_config
        if config_dir.exists():
            yield from config_dir.glob("*.pdb")


def run_srctool(pdb_path: Path, source_root: Path) -> tuple[list[Path], str | None]:
    """
    Run srctool -r on a PDB file to get indexed source files.

    Args:
        pdb_path: Path to the PDB file.
        source_root: Root path to filter source files (only files under this path are included).

    Returns:
        Tuple of (list of unique C++ source file Paths within source_root, error message or None).
    """
    try:
        result = subprocess.run(
            ["srctool", "-r", str(pdb_path)],
            capture_output=True,
            text=True,
            check=False,  # Don't raise on non-zero exit code
        )

        # Resolve source root for comparison
        source_root_resolved = source_root.resolve()

        # srctool outputs source file paths, one per line
        # Filter to only existing C++ files within source_root and return unique paths
        source_files: set[Path] = set()
        for line in result.stdout.strip().splitlines():
            line = line.strip()
            if not line:
                continue
            try:
                path = Path(line)
                if not path.is_file():
                    continue
                if path.suffix.lower() not in CPP_EXTENSIONS:
                    continue
                resolved = path.resolve()
                if resolved.is_relative_to(source_root_resolved):
                    source_files.add(resolved)
            except (OSError, ValueError):
                # Skip invalid paths (too long, permission issues, etc.)
                continue

        error = result.stderr.strip() if result.returncode != 0 else None
        return list(source_files), error

    except FileNotFoundError:
        return (
            [],
            "srctool not found. Ensure Windows SDK is installed and srctool is in PATH.",
        )
    except Exception as e:
        return [], str(e)


def get_pdb_source_info(pdb_path: Path, source_root: Path) -> tuple[PdbSourceInfo, str | None]:
    """
    Get source file information from a PDB, filtering to source root.

    Args:
        pdb_path: Path to the PDB file.
        source_root: Root path to filter source files (only files under this path are included).

    Returns:
        Tuple of (PdbSourceInfo with the indexed source files, error message or None).
    """
    source_files, error = run_srctool(pdb_path, source_root)
    return PdbSourceInfo(pdb_path=pdb_path, source_files=source_files), error


def generate_srcsrv_stream(
    source_files: list[Path],
    source_root: Path,
    source_base_url: str,
    version: str,
) -> str:
    """
    Generate a srcsrv stream file content.

    Args:
        source_files: List of absolute paths to source files.
        source_root: Root path of the source directory.
        source_base_url: Base URL for source files (e.g., https://sources.zeroc.com/ice).
        version: Version string for the URL path.

    Returns:
        The srcsrv stream content as a string.
    """
    # Build the target URL template
    # Remove trailing slash from base URL if present
    base_url = source_base_url.rstrip("/")
    target_url = f"{base_url}/{version}/%var2%"

    # Get current datetime in UTC
    datetime_str = datetime.now(timezone.utc).strftime("%Y-%m-%d %H:%M:%S UTC")

    # Resolve source root for relative path calculation
    source_root_resolved = source_root.resolve()

    # Build source file entries
    source_entries: list[str] = []
    for file_path in sorted(source_files):
        # Get relative path with forward slashes for URL
        rel_path = file_path.relative_to(source_root_resolved).as_posix()
        # Entry format: <absolute_path>*<relative_path>
        source_entries.append(f"{file_path}*{rel_path}")

    # Build the srcsrv stream
    lines = [
        "SRCSRV: ini ------------------------------------------------",
        "VERSION=2",
        f"DATETIME={datetime_str}",
        "SRCSRV: variables ------------------------------------------",
        f"SRCSRVTRG={target_url}",
        "SRCSRVVERCTRL=http",
        "SRCSRVCMD=",
        "SRCSRV: source files ---------------------------------------",
        *source_entries,
        "SRCSRV: end ------------------------------------------------",
    ]

    return "\n".join(lines)


def run_pdbstr(pdb_path: Path, srcsrv_stream: str) -> str | None:
    """
    Write srcsrv stream to a PDB file using pdbstr.

    Args:
        pdb_path: Path to the PDB file to update.
        srcsrv_stream: The srcsrv stream content to write.

    Returns:
        Error message if failed, None on success.
    """
    try:
        # Write stream content to a temporary file
        with tempfile.NamedTemporaryFile(mode="w", suffix=".txt", delete=False, encoding="utf-8") as f:
            f.write(srcsrv_stream)
            stream_file = Path(f.name)

        try:
            # Run pdbstr to write the srcsrv stream to the PDB
            # -w: write mode
            # -s:srcsrv: stream name
            # -p: PDB path
            # -i: input stream file
            result = subprocess.run(
                [
                    "pdbstr",
                    "-w",
                    "-s:srcsrv",
                    f"-p:{pdb_path}",
                    f"-i:{stream_file}",
                ],
                capture_output=True,
                text=True,
                check=False,
            )

            if result.returncode != 0:
                return result.stderr.strip() or f"pdbstr failed with code {result.returncode}"
            return None

        finally:
            # Clean up temporary file
            stream_file.unlink(missing_ok=True)

    except FileNotFoundError:
        return "pdbstr not found. Ensure Windows SDK is installed and pdbstr is in PATH."
    except Exception as e:
        return str(e)


def copy_source_files(
    source_files: set[Path],
    source_root: Path,
    output_sources_dir: Path,
    *,
    verbose: bool = False,
) -> int:
    """
    Copy source files to output directory, preserving relative paths.

    Args:
        source_files: Set of absolute paths to source files.
        source_root: Root path of the source directory.
        output_sources_dir: Directory to copy source files to.
        verbose: If True, print progress information.

    Returns:
        Number of files copied.
    """
    source_root_resolved = source_root.resolve()
    copied_count = 0

    for source_path in sorted(source_files):
        try:
            rel_path = source_path.relative_to(source_root_resolved)
            dest_path = output_sources_dir / rel_path
            dest_path.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(source_path, dest_path)
            copied_count += 1
            if verbose:
                print(f"  Copied: {rel_path}")
        except (ValueError, OSError) as e:
            if verbose:
                print(f"  Failed to copy {source_path}: {e}")

    return copied_count


def copy_and_index_pdb(
    pdb_info: PdbSourceInfo,
    pdbs_dir: Path,
    source_root: Path,
    source_base_url: str,
    version: str,
    *,
    verbose: bool = False,
) -> tuple[Path | None, str | None]:
    """
    Copy a PDB to output directory and write srcsrv stream to it.

    Args:
        pdb_info: PdbSourceInfo with source files for this PDB.
        pdbs_dir: Directory to copy PDB to (the 'pdbs' subdirectory).
        source_root: Root path of the source directory.
        source_base_url: Base URL for source files.
        version: Version string for the URL path.
        verbose: If True, print progress information.

    Returns:
        Tuple of (output PDB path or None, error message or None).
    """
    if not pdb_info.source_files:
        return None, "No source files indexed"

    # Calculate relative path from source root to PDB
    # e.g., cpp/bin/x64/Debug/Ice.pdb -> x64/Debug/Ice.pdb
    try:
        bin_dir = source_root / "cpp" / "bin"
        rel_path = pdb_info.pdb_path.relative_to(bin_dir)
    except ValueError:
        rel_path = pdb_info.pdb_path.name

    # Create output subdirectory
    output_pdb_path = pdbs_dir / rel_path
    output_pdb_path.parent.mkdir(parents=True, exist_ok=True)

    # Copy PDB to output directory
    if verbose:
        print(f"  Copying to {output_pdb_path}")
    shutil.copy2(pdb_info.pdb_path, output_pdb_path)

    # Generate srcsrv stream for this PDB's source files
    srcsrv_content = generate_srcsrv_stream(
        source_files=pdb_info.source_files,
        source_root=source_root,
        source_base_url=source_base_url,
        version=version,
    )

    # Write srcsrv stream to PDB
    if verbose:
        print(f"  Writing srcsrv stream ({len(pdb_info.source_files)} files)")
    error = run_pdbstr(output_pdb_path, srcsrv_content)

    if error:
        return None, error

    return output_pdb_path, None


def collect_all_sources(
    source_dir: Path, *, verbose: bool = False
) -> tuple[set[Path], list[PdbSourceInfo], str | None]:
    """
    Collect all unique source files from PDBs in the Ice build.

    Args:
        source_dir: Path to the Ice source directory.
        verbose: If True, print progress information.

    Returns:
        Tuple of (set of unique source file paths, list of PdbSourceInfo for each PDB, error or None).
    """
    all_sources: set[Path] = set()
    pdb_infos: list[PdbSourceInfo] = []

    pdb_files = list(find_pdb_files(source_dir))

    if verbose:
        print(f"Found {len(pdb_files)} PDB file(s)")

    for pdb_path in pdb_files:
        if verbose:
            print(f"Processing: {pdb_path.name}...", end=" ", flush=True)

        info, error = get_pdb_source_info(pdb_path, source_dir)
        if error:
            if verbose:
                print(f"ERROR: {error}")
            return set(), [], f"{pdb_path.name}: {error}"

        pdb_infos.append(info)
        all_sources.update(info.source_files)
        if verbose:
            print(f"found {len(info.source_files)} source file(s)")

    return all_sources, pdb_infos, None


def main() -> int:
    """Main entry point."""
    args = parse_args()

    source_dir: Path = args.source_dir
    output_dir: Path = args.output_dir

    if not source_dir.exists():
        print(f"Error: Source directory does not exist: {source_dir}", file=sys.stderr)
        return 1

    # Create output subdirectories with version
    sources_dir = output_dir / "sources" / args.version
    pdbs_dir = output_dir / "pdbs" / args.version
    sources_dir.mkdir(parents=True, exist_ok=True)
    pdbs_dir.mkdir(parents=True, exist_ok=True)

    try:
        all_sources, pdb_infos, error = collect_all_sources(source_dir, verbose=args.verbose)
        if error:
            print(f"Error: {error}", file=sys.stderr)
            return 1

        # Print summary
        print(f"Processed {len(pdb_infos)} PDB file(s)")
        print(f"Found {len(all_sources)} unique source file(s) in source root")

        # Filter out PDBs with no source files
        valid_pdbs = [info for info in pdb_infos if info.source_files]

        # Copy source files
        print(f"\nCopying {len(all_sources)} source file(s) to {sources_dir}")
        copied_sources = copy_source_files(
            source_files=all_sources,
            source_root=source_dir,
            output_sources_dir=sources_dir,
            verbose=args.verbose,
        )
        print(f"Copied {copied_sources} source file(s)")

        # Copy and index each PDB
        print(f"\nIndexing {len(valid_pdbs)} PDB(s) to {pdbs_dir}")
        indexed_count = 0
        index_errors: list[tuple[str, str]] = []

        for pdb_info in valid_pdbs:
            if args.verbose:
                print(f"\nProcessing: {pdb_info.pdb_path.name}")

            _, error = copy_and_index_pdb(
                pdb_info=pdb_info,
                pdbs_dir=pdbs_dir,
                source_root=source_dir,
                source_base_url=args.source_base_url,
                version=args.version,
                verbose=args.verbose,
            )

            if error:
                index_errors.append((pdb_info.pdb_path.name, error))
                print(f"  ERROR: {pdb_info.pdb_path.name}: {error}")
            else:
                indexed_count += 1
                if not args.verbose:
                    print(f"  Indexed: {pdb_info.pdb_path.name}")

        # Print final summary
        print(f"\nSuccessfully indexed {indexed_count} PDB(s)")
        if index_errors:
            print(f"Failed to index {len(index_errors)} PDB(s):")
            for name, error in index_errors:
                print(f"  {name}: {error}")
            return 1

        return 0

    except FileNotFoundError as e:
        print(f"Error: {e}", file=sys.stderr)
        return 1
    except Exception as e:
        print(f"Unexpected error: {e}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
