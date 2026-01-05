#!/bin/bash

# This script builds the ZeroC Ice RPM packages from the source code in the zeroc-ice/ice repository.
#
# If ICE_VERSION is set, the script updates the Version field in the ice.spec file accordingly.
# This is typically used for nightly builds.
#
# If ICE_VERSION is not set, the version defined in ice.spec is used.
# This is typically used for release builds.
#
# The build-rpm-packages GitHub Actions workflow in this repository uses this script together
# with the ghcr.io/zeroc-ice/ice-rpm-builder-<distribution>:<channel> Docker image to build the packages.

set -euo pipefail

# Define build root directory
RPM_BUILD_ROOT="/workspace/build"

# Ensure necessary directories exist
mkdir -p "$RPM_BUILD_ROOT"/{BUILD,BUILDROOT,RPMS,SOURCES,SPECS,SRPMS}

# Copy Ice spec file
ICE_SPEC_SRC="/workspace/ice/packaging/rpm/ice.spec"
ICE_SPEC_DEST="$RPM_BUILD_ROOT/SPECS/ice.spec"

cp "$ICE_SPEC_SRC" "$ICE_SPEC_DEST"

# If ICE_VERSION is set, update the version in the spec file to the given version. Otherwise use ICE_VERSION from the
# spec file.
if [[ -n "${ICE_VERSION:-}" ]]; then
    sed -i "s/^Version:.*/Version: $ICE_VERSION/" "$ICE_SPEC_DEST"
fi

# Write common RPM macros to ~/.rpmmacros for rpmbuild and rpmsign
cat > ~/.rpmmacros <<EOF
%vendor ZeroC, Inc.
EOF

# spectool doesn't read ~/.rpmmacros, so we need to pass these macros explicitly.
RPM_MACROS=(--define "_topdir $RPM_BUILD_ROOT")
if [[ -n "${GIT_TAG:-}" ]]; then
    RPM_MACROS+=(--define "git_tag $GIT_TAG")
fi

# Download sources
cd "$RPM_BUILD_ROOT/SOURCES"
spectool -g "${RPM_MACROS[@]}" "$ICE_SPEC_DEST" || { echo "Error: Failed to download sources."; exit 1; }

# Build source RPM
rpmbuild -bs "${RPM_MACROS[@]}" "$ICE_SPEC_DEST"

# Build binary RPM
rpmbuild -bb "${RPM_MACROS[@]}" "$ICE_SPEC_DEST"
