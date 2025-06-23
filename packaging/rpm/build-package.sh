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

# If ICE_VERSION is set, update the version in the spec file to the given version. Otherwise use ICE_VERSION from the
# spec file.
if [[ -n "${ICE_VERSION:-}" ]]; then
    sed -i "s/^Version:.*/Version: $ICE_VERSION/" "$ICE_SPEC_DEST"
fi

# Validate TARGET_ARCH
VALID_ARCHS=("x86_64" "aarch64")
if [[ -z "${TARGET_ARCH:-}" || ! " ${VALID_ARCHS[@]} " =~ " ${TARGET_ARCH} " ]]; then
    echo "Error: TARGET_ARCH is not set or invalid. Use one of: ${VALID_ARCHS[*]}"
    exit 1
fi

# Define common RPM macros
RPM_MACROS=()
RPM_MACROS+=(--define "_topdir $RPM_BUILD_ROOT")
RPM_MACROS+=(--define "vendor ZeroC, Inc.")

if [[ -n "${GIT_TAG:-}" ]]; then
    RPM_MACROS+=(--define "git_tag $GIT_TAG")
fi

# Download sources
cd "$RPM_BUILD_ROOT/SOURCES"
spectool -g "${RPM_MACROS[@]}" "$ICE_SPEC_DEST" || { echo "Error: Failed to download sources."; exit 1; }

# Build source RPM
rpmbuild -bs "$ICE_SPEC_DEST" "${RPM_MACROS[@]}" --target="$TARGET_ARCH"

# Build binary RPM
rpmbuild -bb "$ICE_SPEC_DEST" "${RPM_MACROS[@]}" --target="$TARGET_ARCH"
