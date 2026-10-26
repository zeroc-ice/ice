#!/bin/bash

# This script creates or updates a DNF/YUM repository for ZeroC Ice RPM packages.
#
# --staging specifies the directory containing the built RPM packages.
# --repository specifies the directory of the repository. Each of its x86_64, aarch64, and SRPMS directories holds the
#   current metadata, if any, and a copy of each published package that is staged again or that the metadata doesn't
#   list. The other published packages are not needed: their metadata entries are reused.
# --published specifies a directory with one file per repository directory (x86_64.txt, aarch64.txt, and SRPMS.txt)
#   listing the file names of the packages currently published in it.
#
# The GPG key used to sign the repository must be provided via the GPG_KEY environment variable,
# and the key ID via GPG_KEY_ID.
#
# The publish-rpm-packages GitHub Actions workflow in this repository uses this script together
# with the ghcr.io/zeroc-ice/rpm-repo-builder:<channel> Docker image to create and update the repository.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Source shared GPG setup
source "${SCRIPT_DIR}/../common/setup-gpg.sh"

# Default values
STAGING=""
REPODIR=""
PUBLISHED=""

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
        --published)
            PUBLISHED="$2"
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
: "${PUBLISHED:?Missing --published}"

# Import and validate GPG key
setup_gpg

# Set up ~/.rpmmacros for rpmsign
cat > ~/.rpmmacros <<EOF
%_signature gpg
%_gpg_name $GPG_KEY_ID
%_gpg_path ~/.gnupg
%__gpg_check_password_cmd /bin/true
%__gpg /usr/bin/gpg
EOF

# Rebuilds the metadata of a repository directory and signs it. The metadata lists the published packages and the
# packages in the directory.
update_metadata() {
  local dir="$1"
  local pkglist
  pkglist="$(mktemp)"
  { cat "$PUBLISHED/$(basename "$dir").txt"; find "$dir" -maxdepth 1 -name "*.rpm" -printf "%f\n"; } | sort -u > "$pkglist"

  echo "Running createrepo_c in $dir..."
  # --skip-stat reuses the existing entry of a package by file name, so a published package needs no local copy.
  createrepo_c --update --skip-stat --pkglist "$pkglist" "$dir"
  rm "$pkglist"

  echo "Signing repomd.xml for $(basename "$dir")..."
  gpg --detach-sign --armor --yes --batch "$dir/repodata/repomd.xml"
}

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

  update_metadata "$REPODIR/$arch"
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

update_metadata "$REPODIR/SRPMS"

echo "Repository sync and signing complete."
