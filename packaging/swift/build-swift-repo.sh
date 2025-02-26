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

# Get Git repository root directory
root_dir=$(git rev-parse --show-toplevel)
cd "$root_dir"/packaging/swift

git_hash=$(git rev-parse HEAD)

[ -d ice-swift ] && rm -rf ice-swift
mkdir ice-swift
pushd ice-swift
git init .
git remote add origin https://github.com/zeroc-ice/ice
git fetch --depth=1 origin "$git_hash"
git sparse-checkout set cpp/src/Ice cpp/src/Slice cpp/src/slice2swift cpp/include/Ice swift slice Package.swift
git checkout FETCH_HEAD
rm -rf .git
popd

#
# Perform cleanup on the Package.swift file:
# - Remove exclude paths for cpp build dirs. "src/.../build"
# - Replace XCFramework paths with the URL and checksum
#

# Remove exclude paths for cpp build dirs
sed -i '' -e '/\/build",/d' ice-swift/Package.swift
sed -i '' -e '/"test",/d' ice-swift/Package.swift

# Download each XCFamework, compute its SHA256, and update Package.swift
for name in Ice IceDiscovery IceLocatorDiscovery; do
    zip_name=$name-$ice_version.xcframework.zip
    zip_url=$repository_url/$zip_name

    curl -fsSL -o "${zip_name}" "$repository_url/$zip_name"
    checksum=$(shasum -a 256 "${zip_name}" | cut -d ' ' -f 1)
    indent=$(printf "%12s" "") # indentation for the checksum line

    # replace 'path: "cpp/lib/XCFrameworks/$name.xcframework"' with 'url: "$zip_url", checksum: "$checksum"'
    sed -i '' -e "s|path: \".*$name\.xcframework\"|url: \"${zip_url}\",\n${indent}checksum: \"${checksum}\"|" ice-swift/Package.swift
done

cd ice-swift
git init .
git add .
git config user.name "ZeroC"
git config user.email "git@zeroc.com"
git commit -m "ice: $ice_version"
git tag -a "$ice_version" -m "ice: $ice_version"
