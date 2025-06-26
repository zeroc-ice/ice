#!/bin/bash

# This script builds an RPM package for the ZeroC Ice repository configuration.
#
# --distribution specifies the target distribution (e.g., el9, el10, or amzn2023).
# --channel specifies the Ice version channel (e.g., 3.8 or nightly).
#
# The GPG key used to sign the package must be provided via the GPG_KEY environment variable,
# and the key ID via GPG_KEY_ID.
#
# The resulting package installs a zeroc-ice-<channel>.repo file into /etc/yum.repos.d/.
#
# The build-rpm-ice-repo-packages GitHub Actions workflow in this repository uses this script
# together with the ghcr.io/zeroc-ice/ice-rpm-builder-<distribution> Docker image to build the package.

set -euo pipefail

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

# Ensure GPG_KEY and GPG_KEY_ID are set
: "${GPG_KEY:?GPG_KEY environment variable is not set}"
: "${GPG_KEY_ID:?GPG_KEY_ID environment variable is not set}"

# mutt GPG tty setting
export GPG_TTY=$(tty)

# Import the GPG key
echo "$GPG_KEY" | gpg --batch --import

# Check that the key was successfully imported
if ! gpg --list-secret-keys "$GPG_KEY_ID" > /dev/null 2>&1; then
  echo "Error: GPG key ID $GPG_KEY_ID was not imported successfully."
  exit 1
fi

# Define build root directory
RPM_BUILD_ROOT="/workspace/build"

# Ensure necessary directories exist
mkdir -p "$RPM_BUILD_ROOT"/{BUILD,BUILDROOT,RPMS,SOURCES,SPECS,SRPMS}

# Copy spec file
SPEC_SRC="/workspace/ice/packaging/rpm/ice-repo-$CHANNEL.spec"
SPEC_DEST="$RPM_BUILD_ROOT/SPECS/ice-repo-$CHANNEL.spec"
cp "$SPEC_SRC" "$SPEC_DEST"

# Set up ~/.rpmmacros for rpmbuild and rpmsign
cat > ~/.rpmmacros <<EOF
%_signature gpg
%_gpg_name $GPG_KEY_ID
%_gpg_path ~/.gnupg
%__gpg_check_password_cmd /bin/true
%__gpg /usr/bin/gpg

# Custom build definitions
%_topdir $RPM_BUILD_ROOT
%vendor ZeroC, Inc.
EOF

# Generate the target .repo file from template
REPO_TARGET="$RPM_BUILD_ROOT/SOURCES/zeroc-ice-$CHANNEL.repo"
cp "/workspace/ice/packaging/rpm/zeroc-ice.repo.in" "$REPO_TARGET"
sed -i "s/@CHANNEL@/$CHANNEL/g" "$REPO_TARGET"
sed -i "s/@DISTRIBUTION@/$DISTRIBUTION/g" "$REPO_TARGET"

# Build source RPM
rpmbuild -bs "$SPEC_DEST"

# Build binary RPM
rpmbuild -bb "$SPEC_DEST"

# Sign all RPMs
find "$RPM_BUILD_ROOT" -type f -name "*.rpm" -exec rpmsign --addsign {} +
