#!/bin/bash
set -eux  # Exit on error, print commands

# Create a new tap for the packaging formula
if brew tap | grep -q zeroc-ice/packaging; then
    echo "Packaging tap zeroc-ice/packaging already exists"
else
    echo "Adding packaging tap zeroc-ice/packaging"
    brew tap-new --no-git zeroc-ice/packaging
fi

tap_dir=$(brew --repo zeroc-ice/packaging)
archive_path=$tap_dir/ice.tar.gz

# Create archive of the current git repository
cd "$(git rev-parse --show-toplevel)"
git archive --format=tar.gz -o "$archive_path" HEAD
archive_hash=$(shasum -a 256 "$archive_path" | cut -d ' ' -f 1)

if [ -z "${ICE_VERSION:-}" ]; then
    echo "ICE_VERSION env variable is not set"
    exit 1
fi

export ICE_URL=file://$archive_path
export ICE_URL_SHA256=$archive_hash

envsubst < packaging/brew/ice.rb > "$tap_dir/Formula/ice.rb"

brew uninstall ice || true
brew install --formula --build-bottle zeroc-ice/packaging/ice
brew bottle zeroc-ice/packaging/ice
