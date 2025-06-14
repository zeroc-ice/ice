#!/bin/bash
set -eux -o pipefail

usage() {
    echo "Usage: $0 <ice_version> <repository_url>"
}

ice_version=${1:-}
if [ -z "$ice_version" ]; then
    usage
    exit 1
fi

repository_url=${2:-}
if [ -z "$repository_url" ]; then
    usage
    exit 1
fi

if [ -z "${ICE_NIGHTLY_PUBLISH_TOKEN:-}" ]; then
    echo "Error: ICE_NIGHTLY_PUBLISH_TOKEN environment variable is not set"
    exit 1
fi

# Get Git repository root directory
root_dir=$(git rev-parse --show-toplevel)
cd "$root_dir/packaging/swift"

# Clone the ice-swift-nightly repository and add the Ice for Swift sources
[ -d ice-swift-nightly ] && rm -rf ice-swift-nightly
git clone "https://x-access-token:${ICE_NIGHTLY_PUBLISH_TOKEN}@github.com/zeroc-ice/ice-swift-nightly.git" -b main
cd ice-swift-nightly

cp -rfv ../../../cpp .
cp -rfv ../../../slice .
cp -rfv ../../../swift .
cp -v ../../../Package.swift .

# Download each XCFramework, compute its SHA256, and update Package.swift
for name in Ice IceDiscovery IceLocatorDiscovery; do
    zip_name=$name-$ice_version.xcframework.zip
    zip_url=$repository_url/$zip_name

    curl -fsSL -o "${zip_name}" "$zip_url"
    checksum=$(shasum -a 256 "${zip_name}" | cut -d ' ' -f 1)
    indent=$(printf "%12s" "") # indentation for the checksum line

    # Replace path: "..." with url/checksum lines
    sed -i '' -e "s|path: \".*$name\.xcframework\"|url: \"${zip_url}\",\n${indent}checksum: \"${checksum}\"|" Package.swift
done

git add Package.swift cpp swift README.md
git config user.name "ZeroC"
git config user.email "git@zeroc.com"
git commit -m "ice: $ice_version Nightly build"
git tag -a "$ice_version" -m "ice: $ice_version"
git push origin main --tags
