#!/bin/bash

# This script signs the ZeroC Ice RPM packages build from the zeroc-ice/ice repository.
#
# The GPG key used to sign the packages must be provided via the GPG_KEY environment variable,
# and the key ID via GPG_KEY_ID.
#
# The build-rpm-packages GitHub Actions workflow in this repository uses this script together
# with the ghcr.io/zeroc-ice/ice-rpm-builder-<distribution>:<channel> Docker image to sign the packages.

set -euo pipefail

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

# Write common RPM macros to ~/.rpmmacros for rpmbuild and rpmsign
cat > ~/.rpmmacros <<EOF
%_signature gpg
%_gpg_name $GPG_KEY_ID
%_gpg_path ~/.gnupg
%__gpg_check_password_cmd /bin/true
%__gpg /usr/bin/gpg
%vendor ZeroC, Inc.
EOF

# Sign all RPMs
find "$RPM_BUILD_ROOT" -type f -name "*.rpm" -exec rpmsign --addsign {} +
