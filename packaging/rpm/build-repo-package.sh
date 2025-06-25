#!/bin/bash
set -eux  # Exit on error, print commands

# Required environment: GPG_KEY, GPG_KEY_ID

# Default values
DISTRIBUTION=""
CHANNEL=""

# Parse arguments
while [[ $# -gt 0 ]]; do
    case "$1" in
        --distribution)
            DISTRIBUTION="$2"
            shift 2
            ;;
        --channel)
            CHANNEL="$2"
            shift 2
            ;;
        *)
            echo "Unknown argument: $1"
            exit 1
            ;;
    esac
done

# Validate required inputs
: "${DISTRIBUTION:?Missing --distribution}"
: "${CHANNEL:?Missing --channel}"

# Validate distribution
case "$DISTRIBUTION" in
    el9|el10|amzn2023) ;;
    *)
        echo "Error: DISTRIBUTION must be 'el9', 'el10', or 'amzn2023'" >&2
        exit 1
        ;;
esac

# Validate channel
case "$CHANNEL" in
    3.8|nightly) ;;
    *)
        echo "Error: CHANNEL must be '3.8' or 'nightly'" >&2
        exit 1
        ;;
esac

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
sed -i "s/@DISTRIBUTION@/$DISTRIBUTION/g" "$REPO_TARGET"

# Build source RPM
rpmbuild -bs "$SPEC_DEST" "${RPM_MACROS[@]}"

# Build binary RPM
rpmbuild -bb "$SPEC_DEST" "${RPM_MACROS[@]}"
