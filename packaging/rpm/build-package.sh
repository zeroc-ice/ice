#!/bin/bash
set -eux  # Exit on error, print commands

# Define build root directory
RPM_BUILD_ROOT="/workspace/build"

# Ensure necessary directories exist
mkdir -p "$RPM_BUILD_ROOT"/{BUILD,BUILDROOT,RPMS,SOURCES,SPECS,SRPMS}

# Copy Ice spec file
ICE_SPEC_SRC="/workspace/ice/packaging/rpm/ice.spec"
ICE_SPEC_DEST="$RPM_BUILD_ROOT/SPECS/ice.spec"

cp "$ICE_SPEC_SRC" "$ICE_SPEC_DEST"

# Download sources
cd "$RPM_BUILD_ROOT/SOURCES"
spectool -g "$ICE_SPEC_DEST" || { echo "Error: Failed to download sources."; exit 1; }

# Build source and binary RPMs
rpmbuild -bs "$ICE_SPEC_DEST" -D "_topdir $RPM_BUILD_ROOT"
rpmbuild -bb "$ICE_SPEC_DEST" -D "_topdir $RPM_BUILD_ROOT"
