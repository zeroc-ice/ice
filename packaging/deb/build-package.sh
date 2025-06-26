#!/bin/bash

# This script builds the ZeroC Ice DEB packages from the source code in the zeroc-ice/ice repository.
#
# If ICE_VERSION is set, the script updates the changelog to use that version.
# This is typically used for nightly builds.
#
# If ICE_VERSION is not set, the version from the existing changelog is used.
# This is typically used for release builds.
#
# The build-deb-packages GitHub Actions workflow in this repository uses this script together
# with the ghcr.io/zeroc-ice/ice-deb-builder-<distribution> Docker image to build the packages.

set -euo pipefail

# Create build directory and copy Debian packaging files
mkdir -p /workspace/build
cp -rfv /workspace/ice/packaging/deb/debian /workspace/build

# If ICE_VERSION is set, update the changelog entry to the given version. Otherwise use ICE_VERSION from the changelog.
if [[ -n "${ICE_VERSION:-}" ]]; then
    echo "Updating UNRELEASED changelog entry to: ${ICE_VERSION}"
    dch --changelog /workspace/build/debian/changelog --newversion "$ICE_VERSION" \
        --force-bad-version --distribution UNRELEASED \
        "Ice Nightly build $ICE_VERSION"
else
    ICE_VERSION=$(dpkg-parsechangelog --file /workspace/build/debian/changelog --show-field Version)
fi

UPSTREAM_VERSION=$(echo $ICE_VERSION | cut -f1 -d'-')

# Generate a tarball of the current repository state for the given UPSTREAM_VERSION
echo "Creating tarball for UPSTREAM_VERSION=$UPSTREAM_VERSION"
cd /workspace/ice
git config --global --add safe.directory /workspace/ice
git archive --format=tar.gz -o /workspace/zeroc-ice_${UPSTREAM_VERSION}.orig.tar.gz HEAD

# Unpack the source tarball
cd /workspace/build
tar xzf ../zeroc-ice_${UPSTREAM_VERSION}.orig.tar.gz

# Build the source package (-S generates .dsc and .tar.gz files)
dpkg-buildpackage -S

# Build the binary packages (-b creates .deb files, -uc -us skips signing)
dpkg-buildpackage -b -uc -us
