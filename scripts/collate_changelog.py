#!/usr/bin/env python3
# Copyright (c) ZeroC, Inc.

"""
Collate the changelog fragments under changelog.d/ into the changelog file, then delete the consumed fragments.

The generated section headings are derived from the fragment locations (changelog.d/<section>/<id>.md); the fragment
contents are inserted verbatim. This script does not touch git and does not update the table of contents: review the
result, reorder entries as needed, regenerate the TOC in your editor, and commit.
"""

import argparse
import re
import sys
from pathlib import Path

# Ordered (directory, heading) taxonomy. Fragment directories under changelog.d/ must appear here; sections
# without fragments are omitted from the output.
LANGUAGE_SECTIONS = [
    ("general", "General Changes"),
    ("slice", "Slice Language Changes"),
    ("cpp", "C++ Changes"),
    ("csharp", "C# Changes"),
    ("java", "Java Changes"),
    ("js", "JavaScript Changes"),
    ("matlab", "MATLAB Changes"),
    ("php", "PHP Changes"),
    ("python", "Python Changes"),
    ("ruby", "Ruby Changes"),
    ("swift", "Swift Changes"),
]

# Rendered as "#### <name>" subsections of "### Ice Service Changes".
SERVICE_SECTIONS = [
    ("datastorm", "DataStorm"),
    ("service-glacier2", "Glacier2"),
    ("service-windows", "Ice Service installed as a Windows Service"),
    ("service-icebox", "IceBox"),
    ("service-icegrid", "IceGrid"),
    ("service-icestorm", "IceStorm"),
]

PACKAGING_SECTION = ("packaging", "Packaging Changes")

FRAGMENTS_PLACEHOLDER_PREFIX = "Unreleased changes are recorded as fragments"


def die(message):
    print(f"error: {message}", file=sys.stderr)
    sys.exit(1)


def read_fragments(changelog_d):
    """Return {section directory name: [fragment text, ...]}, with fragments ordered by filename."""
    if not changelog_d.is_dir():
        die(f"{changelog_d} does not exist")

    known = {name for name, _ in LANGUAGE_SECTIONS + SERVICE_SECTIONS + [PACKAGING_SECTION]}
    fragments = {}
    for path in sorted(changelog_d.iterdir()):
        if path.name == "README.md":
            continue
        if not path.is_dir():
            die(f"unexpected file {path}: fragments belong in a section directory")
        if path.name not in known:
            die(f"unknown section directory {path}: add it to the taxonomy in {Path(__file__).name} first")
        for fragment in sorted(path.glob("*.md")):
            text = fragment.read_text(encoding="utf-8").strip()
            if not text.startswith("- "):
                die(f"{fragment} does not start with a '- ' bullet")
            fragments.setdefault(path.name, []).append(text)
    return fragments


def bullet_lines(text):
    """Return the fragment's lines with a blank line between top-level bullets."""
    lines = []
    for line in text.splitlines():
        if line.startswith("- ") and lines and lines[-1] != "":
            lines.append("")
        lines.append(line)
    return lines


def collate(fragments):
    """Return the collated '### ...' sections as a list of lines."""
    lines = []

    def add_section(heading, texts):
        lines.extend([heading, ""])
        for text in texts:
            lines.extend(bullet_lines(text))
            lines.append("")

    for directory, heading in LANGUAGE_SECTIONS:
        if directory in fragments:
            add_section(f"### {heading}", fragments[directory])

    if any(directory in fragments for directory, _ in SERVICE_SECTIONS):
        lines.extend(["### Ice Service Changes", ""])
        for directory, heading in SERVICE_SECTIONS:
            if directory in fragments:
                add_section(f"#### {heading}", fragments[directory])

    if PACKAGING_SECTION[0] in fragments:
        add_section(f"### {PACKAGING_SECTION[1]}", fragments[PACKAGING_SECTION[0]])

    return lines[:-1]  # drop the trailing blank line


def remove_placeholder(lines):
    """Remove the paragraph pointing readers at changelog.d/, if present."""
    for start, line in enumerate(lines):
        if line.startswith(FRAGMENTS_PLACEHOLDER_PREFIX):
            end = start
            while end < len(lines) and lines[end] != "":
                end += 1
            while end < len(lines) and lines[end] == "":
                end += 1
            return lines[:start] + lines[end:]
    return lines


def insert_sections(changelog, version, sections):
    """Return the changelog lines with the collated sections added to the '## Changes in Ice <version>' section,
    creating that section if necessary."""
    lines = changelog.read_text(encoding="utf-8").splitlines()
    version_headings = [(i, line) for i, line in enumerate(lines) if re.match(r"^## Changes in Ice \S+$", line)]
    heading = f"## Changes in Ice {version}"

    match = [i for i, line in version_headings if line == heading]
    if match:
        # Insert at the end of the existing section, after dropping the fragments placeholder.
        start = match[0]
        end = next((i for i, _ in version_headings if i > start), len(lines))
        section = remove_placeholder(lines[start:end])
        while section and section[-1] == "":
            section.pop()
        return lines[:start] + section + [""] + sections + [""] + lines[end:]
    elif version_headings:
        # Create the section above the most recent release, e.g. running 'collate_changelog.py 3.8.3' on the
        # 3.8 branch inserts '## Changes in Ice 3.8.3' above '## Changes in Ice 3.8.2'.
        start, top = version_headings[0]
        previous = top.removeprefix("## Changes in Ice ")
        created = [heading, "", f"These are the changes since the Ice {previous} release.", "", *sections, ""]
        return lines[:start] + created + lines[start:]
    else:
        die(f"{changelog} contains no '## Changes in Ice <version>' heading")


def main():
    parser = argparse.ArgumentParser(description="Collate changelog.d/ fragments into the changelog.")
    parser.add_argument("version", help="the release version, e.g. 3.9.0")
    parser.add_argument(
        "--file",
        type=Path,
        help="the changelog file to update (default: CHANGELOG-<major>.<minor>.md if it exists, else CHANGELOG.md)",
    )
    parser.add_argument("--dry-run", action="store_true", help="print the collated sections without changing anything")
    parser.add_argument("--keep-fragments", action="store_true", help="do not delete the consumed fragment files")
    args = parser.parse_args()

    if not re.match(r"^\d+\.\d+\.\d+$", args.version):
        die(f"'{args.version}' is not a <major>.<minor>.<patch> version")

    root = Path(__file__).resolve().parent.parent
    fragments = read_fragments(root / "changelog.d")
    if not fragments:
        die("no fragments to collate")
    sections = collate(fragments)

    if args.dry_run:
        print("\n".join(sections))
        return

    changelog = args.file
    if changelog is None:
        major_minor = ".".join(args.version.split(".")[:2])
        changelog = root / f"CHANGELOG-{major_minor}.md"
        if not changelog.exists():
            changelog = root / "CHANGELOG.md"
    if not changelog.exists():
        die(f"{changelog} does not exist")

    lines = insert_sections(changelog, args.version, sections)
    changelog.write_text("\n".join(lines).rstrip("\n") + "\n", encoding="utf-8")

    count = sum(len(texts) for texts in fragments.values())
    print(f"Collated {count} fragment(s) into {changelog}.")

    if not args.keep_fragments:
        for directory in fragments:
            for fragment in (root / "changelog.d" / directory).glob("*.md"):
                fragment.unlink()
        print("Deleted the consumed fragments; review the result and regenerate the TOC before committing.")


if __name__ == "__main__":
    main()
