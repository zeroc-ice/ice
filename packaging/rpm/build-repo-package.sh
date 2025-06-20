#!/bin/bash
set -eux  # Exit on error, print commands

: "${CHANNEL:?Must set CHANNEL}"
: "${PLATFORM:?Must set PLATFORM}"

# Define build root directory
RPM_BUILD_ROOT="/workspace/build"

# Ensure necessary directories exist
mkdir -p "$RPM_BUILD_ROOT"/{BUILD,BUILDROOT,RPMS,SOURCES,SPECS,SRPMS}

# Copy spec file
SPEC_SRC="/workspace/ice/packaging/rpm/ice-repo-$CHANNEL.spec"
SPEC_DEST="$RPM_BUILD_ROOT/SPECS/ice-repo-$CHANNEL.spec"
cp "$SPEC_SRC" "$SPEC_DEST"

# Define RPM macros
RPM_MACROS=()
RPM_MACROS+=(--define "_topdir $RPM_BUILD_ROOT")
RPM_MACROS+=(--define "vendor ZeroC, Inc.")

# Generate the target .repo file from template
REPO_TARGET="$RPM_BUILD_ROOT/SOURCES/zeroc-ice-$CHANNEL.repo"
cp "/workspace/ice/packaging/rpm/zeroc-ice.repo.in" "$REPO_TARGET"
sed -i "s/@CHANNEL@/$CHANNEL/g" "$REPO_TARGET"
sed -i "s/@PLATFORM@/$PLATFORM/g" "$REPO_TARGET"

# Build source RPM
rpmbuild -bs "$SPEC_DEST" "${RPM_MACROS[@]}"

# Build binary RPM
rpmbuild -bb "$SPEC_DEST" "${RPM_MACROS[@]}"
