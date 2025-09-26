#!/bin/bash
set -eux -o pipefail

usage() {
    echo "Usage: $0"
}

if [ -z "${ICE_NIGHTLY_PUBLISH_TOKEN:-}" ]; then
    echo "Error: ICE_NIGHTLY_PUBLISH_TOKEN environment variable is not set"
    exit 1
fi

if [ -z "${STAGING_DIR:-}" ]; then
    echo "Error: STAGING_DIR environment variable is not set"
    exit 1
fi

# Get Git repository root directory
root_dir=$(git rev-parse --show-toplevel)
cd "$root_dir/packaging/swift"

# Clone the ice-swift-nightly repository and add the Ice for Swift sources
[ -d ice-swift-nightly ] && rm -rf ice-swift-nightly
git clone "https://x-access-token:${ICE_NIGHTLY_PUBLISH_TOKEN}@github.com/zeroc-ice/ice-swift-nightly.git" -b main
cd ice-swift-nightly

# Remove existing directories to avoid keeping stale files that may have been removed from the Ice repository.
rm -rfv cpp slice swift

# Copy the Ice for Swift sources and package manifest.
cp -rfv ../../../cpp .
cp -rfv ../../../slice .
cp -rfv ../../../swift .
cp -v ../../../Package.swift .

# Update the Packages.swift file with the URL and Checksum for the xcframeworks
for zip_file in "${STAGING_DIR}"/*.zip; do
    echo "Processing zip file: ${zip_file}"
    zip_name=$(basename -s ".xcframework.zip" "${zip_file}")
    name="${zip_name%%-*}"
    version="${zip_name#*-}"
    zip_url="https://download.zeroc.com/ice/nightly/${zip_name}.xcframework.zip"

    checksum=$(shasum -a 256 "${zip_file}" | cut -d ' ' -f 1)
    indent=$(printf "%12s" "") # indentation for the checksum line

    # Replace path: "..." with url/checksum lines
    sed -i '' -e "s|path: \".*$name\.xcframework\"|url: \"${zip_url}\",\n${indent}checksum: \"${checksum}\"|" Package.swift
done

# Commit and push the changes
git add .
git config user.name "ZeroC"
git config user.email "git@zeroc.com"
git commit -m "ice: $version Nightly build"
git tag -a "$version" -m "ice: $version"
git push origin main --tags
