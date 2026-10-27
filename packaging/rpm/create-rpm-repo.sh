#!/bin/bash

# This script creates or updates a DNF/YUM repository for ZeroC Ice RPM packages.
#
# --staging specifies the directory containing the built RPM packages.
# --repository specifies the directory of the repository. Each of its x86_64, aarch64, and SRPMS directories holds the
#   current metadata, if any, and a copy of each published package that the metadata doesn't list. The other published
#   packages are not needed: their metadata entries are reused.
# --published specifies a directory with one file per repository directory (x86_64.txt, aarch64.txt, and SRPMS.txt)
#   listing the file names of the packages currently published in it. A staged package with a published name is
#   skipped, so the published copy wins.
#
# The metadata identifies a published package by its file name, so a published package is never replaced. To replace
# one anyway, delete the package and its directory's repodata/ in S3, then publish right away: the repository is
# unusable until the next publish rebuilds its metadata from every package.
#
# The GPG key used to sign the repository must be provided via the GPG_KEY environment variable,
# and the key ID via GPG_KEY_ID.
#
# The publish-rpm-packages GitHub Actions workflow in this repository uses this script together
# with the ghcr.io/zeroc-ice/rpm-repo-builder:<channel> Docker image to create and update the repository.

set -euo pipefail

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

# Copies staged packages into a repository directory, except those already in it or published.
copy_packages() {
  local dir="$1"
  shift
  mkdir -p "$dir"
  for rpm in "$@"; do
    local name
    name="$(basename "$rpm")"
    if [[ -f "$dir/$name" ]] || grep -qxF "$name" "$PUBLISHED/$(basename "$dir").txt"; then
      echo "Skipping existing: $name"
      continue
    fi
    echo "Copying: $name"
    cp "$rpm" "$dir/$name"
  done
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

  copy_packages "$REPODIR/$arch" "${rpms[@]}"
  update_metadata "$REPODIR/$arch"
done

# Process source RPMs
echo "Processing SRPMS (source RPMs)..."
mapfile -t srpms < <(find "$STAGING" -type f -name "*.src.rpm")
copy_packages "$REPODIR/SRPMS" "${srpms[@]}"
update_metadata "$REPODIR/SRPMS"

echo "Repository sync and signing complete."
