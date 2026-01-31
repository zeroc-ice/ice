#!/bin/bash
set -eux -o pipefail

# Build slice2swift artifact bundle for SwiftPM
#
# This script builds slice2swift and creates the artifact bundle using the make build system.
# The bundle is placed at cpp/bin/slice2swift.artifactbundle.
#
# Usage:
#   ./packaging/swift/build-slice2swift-bundle.sh [--zip]
#
# Options:
#   --zip    Also create a zip archive for distribution (slice2swift-<version>.artifactbundle.zip)
#
# Environment:
#   ICE_VERSION    Override the version (default: from config/version.env)

create_zip=false
if [[ "${1:-}" == "--zip" ]]; then
    create_zip=true
fi

# Get Git repository root directory
root_dir=$(git rev-parse --show-toplevel)
cd "$root_dir"

# Load version from config/version.env if not set
if [[ -z "${ICE_VERSION:-}" ]]; then
    source config/version.env
    ICE_VERSION="${VERSION}"
fi

echo "Building slice2swift artifact bundle version ${ICE_VERSION}"

# Build slice2swift and the artifact bundle using make
# The artifact bundle is created automatically on macOS as part of the srcs target
make -C cpp bin/slice2swift.artifactbundle OPTIMIZE=yes

bundle_dir="cpp/bin/slice2swift.artifactbundle"
if [[ ! -d "$bundle_dir" ]]; then
    echo "Error: artifact bundle not found at $bundle_dir"
    exit 1
fi

echo "Created artifact bundle at: $bundle_dir"

# Optionally create zip archive for CI/distribution
if [[ "$create_zip" == true ]]; then
    zip_name="slice2swift-${ICE_VERSION}.artifactbundle.zip"
    zip_path="cpp/bin/$zip_name"
    rm -f "$zip_path"
    (cd cpp/bin && zip -r "$zip_name" slice2swift.artifactbundle)
    echo "Created zip archive at: $zip_path"

    # Print checksum for convenience
    checksum=$(shasum -a 256 "$zip_path" | cut -d ' ' -f 1)
    echo "SHA256 checksum: $checksum"
fi

echo "Done."
