#!/usr/bin/env bash

# This script creates an APT repository for ZeroC Ice DEB packages.
#
# --distribution specifies the target distribution (e.g., debian12, debian13, or ubuntu24.04).
# --channel specifies the Ice version channel (e.g., 3.8 or 3.9).
# --quality specifies the release quality (e.g., stable, or nightly).
# --staging specifies the directory containing the built DEB packages.
# --repository specifies the directory where the APT repository will be created. This can also be
# an existing repository directory if you are updating it.
#
# The GPG key used to sign the repository must be provided via the GPG_KEY environment variable,
# and the key ID via GPG_KEY_ID.
#
# The publish-deb-packages GitHub Actions workflow in this repository uses this script together
# with the ghcr.io/zeroc-ice/deb-repo-builder:<channel> Docker image to create and update the repository.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Source shared scripts
source "${SCRIPT_DIR}/../common/setup-gpg.sh"
source "${SCRIPT_DIR}/../common/codenames.sh"

# Default values
DISTRIBUTION=""
CHANNEL=""
QUALITY=""
STAGING=""
REPODIR=""

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
: "${DISTRIBUTION:?Missing --distribution}"
: "${CHANNEL:?Missing --channel}"
: "${QUALITY:?Missing --quality}"
: "${STAGING:?Missing --staging}"
: "${REPODIR:?Missing --repo}"

# Import and validate GPG key
setup_gpg

# Map distribution to codename
CODENAME="${CODENAMES[$DISTRIBUTION]}"
DIST_DIR="$REPODIR/$DISTRIBUTION"
CONF_DIR="$DIST_DIR/conf"

echo "Creating APT repository layout for $DISTRIBUTION in $DIST_DIR..."

mkdir -p "$CONF_DIR"

# Write conf/options
cat > "$CONF_DIR/options" <<EOF
verbose
basedir .
EOF

# Write conf/distributions
cat > "$CONF_DIR/distributions" <<EOF
Origin: ZeroC
Label: Ice $CHANNEL $QUALITY Repository
Codename: $CODENAME
Suite: stable
Version: $CHANNEL-$QUALITY
Architectures: amd64 arm64 source
Components: main
Description: ZeroC Ice $CHANNEL $QUALITY packages for $DISTRIBUTION
SignWith: $GPG_KEY_ID
EOF

echo "✓ Repository config created at $CONF_DIR"
# Collect binary packages
packages=()
while IFS= read -r file;
    do packages+=("$file");
done < <(find "$STAGING/deb-packages-$DISTRIBUTION-amd64" -type f -name "*_amd64.deb")

while IFS= read -r file; do
    packages+=("$file");
done < <(find "$STAGING/deb-packages-$DISTRIBUTION-arm64" -type f -name "*_arm64.deb")

while IFS= read -r file; do
    packages+=("$file");
done < <(find "$STAGING/deb-packages-$DISTRIBUTION-amd64" -type f -name "*_all.deb")

# Add binary packages
for package in "${packages[@]}"; do
    reprepro -b "$DIST_DIR" includedeb "$CODENAME" "$package"
done

# Collect source packages
src_packages=()
while IFS= read -r file; do
    src_packages+=("$file");
done < <(find "$STAGING/deb-packages-$DISTRIBUTION-amd64" -type f -name "*.dsc")

# Add source packages
for package in "${src_packages[@]}"; do
    reprepro -b "$DIST_DIR" includedsc "$CODENAME" "$package"
done
