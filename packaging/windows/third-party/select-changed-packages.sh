#!/usr/bin/env bash
#
# Prints, as a JSON array, the entries of packages.json that a change affects.
#
# Usage: select-changed-packages.sh [<base-commit>]
#
# The change is the difference between <base-commit> and HEAD. Without a base commit, every package is selected.
#
# A package is selected when the change touches:
#   - a file in the directory of its project,
#   - a patch of its dependency, in patches/<dependency>/, or
#   - the entry of its dependency in sources.json.
#
# README.md selects nothing. Any other file in this directory, and any workflow that builds these packages, is shared
# by all the packages and selects them all.

set -euo pipefail

base="${1:-}"
dir="packaging/windows/third-party"

cd "$(git rev-parse --show-toplevel)"

if [[ -z "$base" ]]; then
    jq -c '.' "$dir/packages.json"
    exit 0
fi

# Without rename detection, a file moved out of a package's directory is listed under both its old and new paths.
changed_files=$(git diff --name-only --no-renames "$base" HEAD -- \
    "$dir" '.github/workflows/*third-party*windows*.yml')

# The dependencies whose sources.json entry was added, modified or removed.
base_sources=$(git show "$base:$dir/sources.json" 2>/dev/null || echo '{}')
changed_sources=$(jq -c --argjson base "$base_sources" \
    '. as $head | [$base + $head | keys[] | select($base[.] != $head[.])]' "$dir/sources.json")

jq -c --arg dir "$dir" --arg files "$changed_files" --argjson sources "$changed_sources" '
    # Whether the input, a file path, belongs to $package rather than being shared by the packages.
    def owned_by($package):
        startswith($package.project | sub("/[^/]*$"; "/")) or
        startswith("\($dir)/patches/\($package.dependency)/");

    . as $packages
    | [$files | split("\n")[] | select(. != "" and . != "\($dir)/README.md" and . != "\($dir)/sources.json")]
        as $files
    | ($files | any(. as $file | $packages | all(. as $package | $file | owned_by($package) | not)))
        as $shared_file_changed
    | map(. as $package | select(
        $shared_file_changed or
        ($sources | index($package.dependency) != null) or
        ($files | any(owned_by($package)))))
' "$dir/packages.json"
