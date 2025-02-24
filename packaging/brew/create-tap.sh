#!/bin/bash
set -eux -o pipefail

# This script is used to create a git repository containing a formula for the nightly version of Ice.
# The formula is then used to build a bottle for the nightly version of Ice.
#
# Note: The current commit hash is used as the commit to build the bottle from. It must exist on the
# zeroc-ice/ice repository.

usage() {
    echo "Usage: $0 <tap_name> <ice_version>"
}

tap_name="${1:-}"

if [ -z "$tap_name" ]; then
    usage
    exit 1
fi

ice_version="${2:-}"

if [ -z "$ice_version" ]; then
    usage
    exit 1
fi

git_hash=$(git rev-parse HEAD)
archive_url=https://github.com/zeroc-ice/ice/archive/${git_hash}.tar.gz

# Download and compute SHA256 of the archive
echo "Downloading $archive_url"
curl -fsSL "$archive_url" -o "ice.tar.gz"

echo "Computing SHA256 of ice.tar.gz"
archive_hash=$(shasum -a 256 "ice.tar.gz" | cut -d ' ' -f 1)
rm ice.tar.gz

tap_path=$(brew --repo $tap_name)

# Create the tap if it does not exist
if [ -d "$tap_path" ]; then
    echo "Tap $tap_name already exists at $tap_path"
else
    echo "Creating tap $tap_name"
    mkdir -p "$tap_path/Formula"
    git init --initial-branch=main "$tap_path"
fi

tap_formula_path=$tap_path/Formula/ice.rb
ice_formula_template=packaging/brew/ice.rb

export ICE_URL=$archive_url
export ICE_VERSION=$ice_version
export ICE_URL_SHA256=$archive_hash

envsubst < "$ice_formula_template" > "$tap_formula_path"

cd "$tap_path"

git config user.name "ZeroC"
git config user.email "git@zeroc.com"
git add Formula/ice.rb # just add the formula we don't want the rest to be included
git commit -m "ice: $ice_version"
