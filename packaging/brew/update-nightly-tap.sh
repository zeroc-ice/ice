#!/bin/bash
set -eux -o pipefail

# This script is used to create a git repository containing a formula for the nightly version of Ice.
# The formula is then used to build a bottle for the nightly version of Ice.
#
# Note: The current commit hash is used as the commit to build the bottle from. It must exist on the
# zeroc-ice/ice repository.

BASE_URL="https://download.zeroc.com/ice"

usage() {
    echo "Usage: $0 <channel> <quality> <ice_version> <icegridgui_dmg_sha256>"
    echo "  <channel>      The Ice channel (e.g., 3.8, 3.9)"
    echo "  <quality>      The release quality (e.g., nightly, stable)"
    echo "  <ice_version>  The version of Ice to build (e.g., 3.9.0-nightly-20231020)"
    echo "  <icegridgui_dmg_sha256>  The SHA256 of the IceGrid GUI DMG"
}

channel="${1:-}"

if [ -z "$channel" ]; then
    usage
    exit 1
fi

quality="${2:-}"

if [ -z "$quality" ]; then
    usage
    exit 1
fi

ice_version="${3:-}"

if [ -z "$ice_version" ]; then
    usage
    exit 1
fi

icegridgui_dmg_sha256="${4:-}"

if [ -z "$icegridgui_dmg_sha256" ]; then
    usage
    exit 1
fi

# Construct the root URL for the bottles
if [ "$quality" = "stable" ]; then
    root_url="${BASE_URL}/${channel}"
else
    root_url="${BASE_URL}/${quality}/${channel}"
fi

git_hash=$(git rev-parse HEAD)
archive_url=https://github.com/zeroc-ice/ice/archive/${git_hash}.tar.gz

# Download and compute SHA256 of the archive
echo "Downloading $archive_url"
curl -fsSL "$archive_url" -o "ice.tar.gz"

echo "Computing SHA256 of ice.tar.gz"
archive_hash=$(shasum -a 256 "ice.tar.gz" | cut -d ' ' -f 1)
rm ice.tar.gz

tap_name="zeroc-ice/nightly"
tap_path=$(brew --repo $tap_name)

# Create the tap if it does not exist
if [ -d "$tap_path" ]; then
    echo "Tap $tap_name already exists at $tap_path"
else
    brew tap "$tap_name"
fi

tap_formula_path=$tap_path/Formula/ice@${channel}.rb
ice_formula_template=packaging/brew/ice.rb
tap_cask_path=$tap_path/Casks/icegridgui@${channel}.rb
icegridgui_cask_template=packaging/brew/icegridgui.rb
icegridgui_dmg_url="${BASE_URL}/${channel}/IceGridGUI-${ice_version}.dmg"

export ICE_URL=$archive_url
export ICE_VERSION=$ice_version
export ICE_URL_SHA256=$archive_hash
export ICE_FORMULA_CLASS="IceAT${channel//./}"
export ICEGRIDGUI_CASK_NAME="icegridgui@${channel}"
export ICEGRIDGUI_DMG_URL=$icegridgui_dmg_url
export ICEGRIDGUI_DMG_SHA256=$icegridgui_dmg_sha256

envsubst < "$ice_formula_template" > "$tap_formula_path"
envsubst < "$icegridgui_cask_template" > "$tap_cask_path"

brew install --build-bottle zeroc-ice/nightly/ice@${channel}
brew bottle zeroc-ice/nightly/ice@${channel} --root-url="$root_url" --json
brew bottle --merge ./ice@${channel}--*.bottle.json --write --no-commit

# Rename file to fix `--`. See https://github.com/orgs/Homebrew/discussions/4541
for file in ice@${channel}--*.bottle.*tar.gz; do
    mv "$file" "${file/--/-}"
done

git -C "$tap_path" config user.name "ZeroC"
git -C "$tap_path" config user.email "git@zeroc.com"

# Add the formula and cask and commit
git -C "$tap_path" add Formula/ice@${channel}.rb Casks/icegridgui@${channel}.rb
git -C "$tap_path" commit -m "ice@${channel}, icegridgui@${channel}: $ice_version"

# Create a patch to attach to the GitHub release
git -C "$tap_path" format-patch -1 HEAD --stdout > "ice@${channel}-$ice_version.patch"
