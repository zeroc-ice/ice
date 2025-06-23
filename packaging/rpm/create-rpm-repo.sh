#!/bin/bash
set -euo pipefail

# Default values
CHANNEL=""
STAGING=""
REPODIR=""

# Parse arguments
while [[ $# -gt 0 ]]; do
    case "$1" in
        --channel)
            CHANNEL="$2"
            shift 2
            ;;
        --staging)
            STAGING="$2"
            shift 2
            ;;
        --repo)
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
: "${CHANNEL:?Missing --channel}"
: "${STAGING:?Missing --staging}"
: "${REPODIR:?Missing --repo}"
: "${GPG_KEY:?GPG_KEY environment variable is not set}"
: "${GPG_KEY_ID:?GPG_KEY_ID environment variable is not set}"

# Validate channel
case "$CHANNEL" in
    3.8|nightly) ;;
    *)
        echo "Error: CHANNEL must be '3.8' or 'nightly'" >&2
        exit 1
        ;;
esac

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

if [ "${CHANNEL}" = "nightly" ]; then
  echo "Pruning RPMs older than 3 days from ${DISTRIBUTION}..."
  for arch in x86_64 aarch64 SRPMS; do
    find "${DISTRIBUTION}/${arch}" -type f -name "*.rpm" -mtime +3 -exec rm -v {} \;
  done
fi

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
