#!/bin/bash

# This script builds an RPM package for the ZeroC Ice repository configuration.
#
# --distribution specifies the target distribution (e.g., el9, el10, or amzn2023).
# --channel specifies the Ice version channel (e.g., 3.9 or 3.8).
# --quality specifies the release quality (e.g., stable, or nightly).
#
# The resulting package installs a zeroc-ice-<channel>.repo file into /etc/yum.repos.d/.
#
# The build-rpm-ice-repo-packages GitHub Actions workflow in this repository uses this script
# together with the ghcr.io/zeroc-ice/ice-rpm-builder-<distribution>:<channel> Docker image to build the package.

set -euo pipefail

DISTRIBUTION=""
CHANNEL=""
QUALITY=""

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
        --quality)
            QUALITY="$2"
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
: "${QUALITY:?Missing --quality}"

# Define package name based on quality
if [[ "$QUALITY" == "stable" ]]; then
    PACKAGE_NAME="ice-repo-$CHANNEL"
    REPO_FILENAME="zeroc-ice-$CHANNEL.repo"
    UPLOAD_PREFIX="$CHANNEL"
else
    PACKAGE_NAME="ice-repo-$CHANNEL-$QUALITY"
    REPO_FILENAME="zeroc-ice-$CHANNEL-$QUALITY.repo"
    UPLOAD_PREFIX="$QUALITY/$CHANNEL"
fi

# Define build root directory
RPM_BUILD_ROOT="/workspace/build"

# Ensure necessary directories exist
mkdir -p "$RPM_BUILD_ROOT"/{BUILD,BUILDROOT,RPMS,SOURCES,SPECS,SRPMS}

# Copy spec file
SPEC_SRC="/workspace/ice/packaging/rpm/$PACKAGE_NAME.spec"
SPEC_DEST="$RPM_BUILD_ROOT/SPECS/$PACKAGE_NAME.spec"
cp "$SPEC_SRC" "$SPEC_DEST"

# Set up ~/.rpmmacros for rpmbuild and rpmsign
cat > ~/.rpmmacros <<EOF
# Custom build definitions
%_topdir $RPM_BUILD_ROOT
%vendor ZeroC, Inc.
EOF

# Generate the target .repo file from template
REPO_TARGET="$RPM_BUILD_ROOT/SOURCES/$REPO_FILENAME"
cp "/workspace/ice/packaging/rpm/zeroc-ice.repo.in" "$REPO_TARGET"
sed -i "s#@CHANNEL@#$CHANNEL#g" "$REPO_TARGET"
sed -i "s#@QUALITY@#$QUALITY#g" "$REPO_TARGET"
sed -i "s#@DISTRIBUTION@#$DISTRIBUTION#g" "$REPO_TARGET"
sed -i "s#@UPLOAD_PREFIX@#${UPLOAD_PREFIX}#g" "$REPO_TARGET"

# Build source RPM
rpmbuild -bs "$SPEC_DEST"

# Build binary RPM
rpmbuild -bb "$SPEC_DEST"
