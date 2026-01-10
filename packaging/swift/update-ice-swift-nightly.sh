#!/bin/bash
set -eux -o pipefail

usage() {
    echo "Usage: $0"
}

if [ -z "${ICE_NIGHTLY_PUBLISH_TOKEN:-}" ]; then
    echo "Error: ICE_NIGHTLY_PUBLISH_TOKEN environment variable is not set"
    exit 1
fi

if [ -z "${CHANNEL:-}" ]; then
    echo "Error: CHANNEL environment variable is not set"
    exit 1
fi

if [ -z "${VERSION:-}" ]; then
    echo "Error: VERSION environment variable is not set"
    exit 1
fi

# Get Git repository root directory
root_dir=$(git rev-parse --show-toplevel)
cd "$root_dir/packaging/swift"

# Clone the ice-swift-nightly repository and add the Ice for Swift sources
# Use the channel (e.g., 3.7, 3.8) as the branch name
[ -d ice-swift-nightly ] && rm -rf ice-swift-nightly
git clone "https://x-access-token:${ICE_NIGHTLY_PUBLISH_TOKEN}@github.com/zeroc-ice/ice-swift-nightly.git" -b "${CHANNEL}"
cd ice-swift-nightly

# Remove existing Sources directory to avoid keeping stale files
rm -rfv Sources cpp slice swift

# Copy the generated SPM sources from build-spm-sources.py output
cp -rfv ../spm/Sources .
cp -v ../spm/Package.swift .
cp -v ../spm/LICENSE .
cp -v ../spm/ICE_LICENSE .
cp -v ../README.md .

# Commit and push the changes
git add .
git config user.name "ZeroC"
git config user.email "git@zeroc.com"
git commit -m "ice: $VERSION Nightly build"
git tag -a "$VERSION" -m "ice: $VERSION"
git push origin "${CHANNEL}" --tags
