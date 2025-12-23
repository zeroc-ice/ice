#!/bin/bash

# This script creates a DNF/YUM repository for ZeroC Ice RPM packages.
#
# --staging specifies the directory containing the built RPM packages.
# --repository specifies the directory where the DNF repository will be created or updated.
#
# The GPG key used to sign the repository must be provided via the GPG_KEY environment variable,
# and the key ID via GPG_KEY_ID.
#
# The publish-rpm-packages GitHub Actions workflow in this repository uses this script together
# with the ghcr.io/zeroc-ice/rpm-repo-builder-<channel> Docker image to create and update the repository.

set -euo pipefail

# Default values
STAGING=""
REPODIR=""

# Parse arguments
while [[ $# -gt 0 ]]; do
    case "$1" in
        --staging)
            STAGING="$2"
            shift 2
            ;;
        --repository)
            REPODIR="$2"
            shift 2
            ;;
        *)
            echo "Unknown argument: $1"
            exit 1
            ;;
    esac
done

# Validate required inputs
: "${STAGING:?Missing --staging}"
: "${REPODIR:?Missing --repository}"
: "${GPG_KEY:?GPG_KEY environment variable is not set}"
: "${GPG_KEY_ID:?GPG_KEY_ID environment variable is not set}"

# Import the GPG key
echo "$GPG_KEY" | gpg --batch --import

# Check that the key was successfully imported
if ! gpg --list-secret-keys "$GPG_KEY_ID" > /dev/null 2>&1; then
  echo "Error: GPG key ID $GPG_KEY_ID was not imported successfully."
  exit 1
fi

# Set up ~/.rpmmacros for rpmsign
cat > ~/.rpmmacros <<EOF
%_signature gpg
%_gpg_name $GPG_KEY_ID
%_gpg_path ~/.gnupg
%__gpg_check_password_cmd /bin/true
%__gpg /usr/bin/gpg
EOF

ARCHES=(x86_64 aarch64)
NOARCH_RPMS=()

echo "Syncing RPMs from '$STAGING' to '$REPODIR'..."

# Collect noarch RPMs once
mapfile -t NOARCH_RPMS < <(find "$STAGING" -type f -name "*.noarch.rpm")

# Process each architecture
for arch in "${ARCHES[@]}"; do
  echo "Processing architecture: $arch"

  # Find arch-specific RPMs
  mapfile -t rpms < <(find "$STAGING" -type f -name "*.${arch}.rpm")

  # Add noarch RPMs
  rpms+=("${NOARCH_RPMS[@]}")

  # Create architecture-specific repo dir
  mkdir -p "$REPODIR/$arch"

  for rpm in "${rpms[@]}"; do
    target="$REPODIR/$arch/$(basename "$rpm")"
    if [[ -f "$target" ]]; then
      echo "Skipping existing: $(basename "$rpm")"
      continue
    fi
    echo "Copying: $(basename "$rpm")"
    cp "$rpm" "$target"
  done

  # Run createrepo_c for this arch
  echo "Running createrepo_c in $REPODIR/$arch..."
  createrepo_c --update "$REPODIR/$arch"

  # Sign repomd.xml
  REPO_MD="$REPODIR/$arch/repodata/repomd.xml"
  if [[ -f "$REPO_MD" ]]; then
    echo "Signing repomd.xml for $arch..."
    gpg --detach-sign --armor --yes --batch "$REPO_MD"
  else
    echo "Warning: repomd.xml not found for $arch!"
    exit 1
  fi
done

# Process source RPMs
echo "Processing SRPMS (source RPMs)..."
mapfile -t srpms < <(find "$STAGING" -type f -name "*.src.rpm")
mkdir -p "$REPODIR/SRPMS"

for srpm in "${srpms[@]}"; do
  target="$REPODIR/SRPMS/$(basename "$srpm")"
  if [[ -f "$target" ]]; then
    echo "Skipping existing: $(basename "$srpm")"
    continue
  fi
  echo "Copying: $(basename "$srpm")"
  cp "$srpm" "$target"
done

echo "Running createrepo_c in $REPODIR/SRPMS..."
createrepo_c --update "$REPODIR/SRPMS"

REPO_MD="$REPODIR/SRPMS/repodata/repomd.xml"
if [[ -f "$REPO_MD" ]]; then
  echo "Signing repomd.xml for SRPMS..."
  gpg --detach-sign --armor --yes --batch "$REPO_MD"
else
  echo "Warning: repomd.xml not found for SRPMS!"
  exit 1
fi

echo "Repository sync and signing complete."
