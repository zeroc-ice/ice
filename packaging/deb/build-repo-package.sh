#!/usr/bin/env bash

# This script builds a DEB package that installs the ZeroC Ice APT repository configuration.
#
# --distribution specifies the target distribution (e.g., debian12 or ubuntu24.04).
# --channel specifies the Ice version channel (e.g., 3.8 or nightly).
#
# The resulting package installs the ice-repo-<channel>.list file into /etc/apt/sources.list.d/
# and the public GPG key into /usr/share/keyrings/zeroc-archive-keyring.gpg.
#
# The GPG key to include in the keyring must be provided via the GPG_KEY environment variable
# (in ASCII-armored format), and the corresponding key ID via GPG_KEY_ID.
#
# The build-deb-ice-repo-packages GitHub Actions workflow in this repository uses this script together
# with the ghcr.io/zeroc-ice/ice-deb-builder-<distribution> Docker image to build the package.

set -euo pipefail

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
: "${GPG_KEY:?GPG_KEY environment variable is not set}"
: "${GPG_KEY_ID:?GPG_KEY_ID environment variable is not set}"

# Validate distribution
case "$DISTRIBUTION" in
    debian12|ubuntu24.04) ;;
    *)
        echo "Error: DISTRIBUTION must be 'debian12' or 'ubuntu24.04'" >&2
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

declare -A CODENAMES=(
    ["debian12"]="bookworm"
    ["ubuntu24.04"]="noble"
)

CODENAME="${CODENAMES[$DISTRIBUTION]}"

# Import the GPG key
echo "$GPG_KEY" | gpg --batch --import

# Check that the key was successfully imported
if ! gpg --list-secret-keys "$GPG_KEY_ID" > /dev/null 2>&1; then
  echo "Error: GPG key ID $GPG_KEY_ID was not imported successfully."
  exit 1
fi

# Configuration
REPO_BASE_URL="https://download.zeroc.com/ice/${CHANNEL}"
OUT_DIR="/workspace/build"
VERSION="1.0"
KEYRING_NAME="zeroc-archive-keyring.gpg"
PACKAGE_NAME="ice-repo-${CHANNEL}"
KEYRING_PATH="usr/share/keyrings/${KEYRING_NAME}"
SOURCE_LIST_PATH="etc/apt/sources.list.d/ice-repo-${CHANNEL}.list"

# Create keyring if needed
mkdir -p "${OUT_DIR}/keyrings"
KEYRING_OUTPUT="${OUT_DIR}/keyrings/${KEYRING_NAME}"
if [[ ! -f "$KEYRING_OUTPUT" ]]; then
  echo "Generating keyring..."
  gpg --export "$GPG_KEY_ID" | gpg --dearmor -o "$KEYRING_OUTPUT"
fi

# Create package layout
PKG_DIR="${OUT_DIR}/${PACKAGE_NAME}"
mkdir -p "${PKG_DIR}/DEBIAN"
mkdir -p "${PKG_DIR}/$(dirname "$SOURCE_LIST_PATH")"
mkdir -p "${PKG_DIR}/$(dirname "$KEYRING_PATH")"

# Create control file
cat > "${PKG_DIR}/DEBIAN/control" <<EOF
Package: ice-repo-${CHANNEL}
Version: ${VERSION}
Architecture: all
Maintainer: ZeroC, Inc. <info@zeroc.com>
Description: ZeroC APT repository configuration for Ice ${CHANNEL}
 This package installs the APT repository and GPG key for the ZeroC Ice ${CHANNEL} repository on ${DISTRIBUTION}.
EOF

# Create sources.list.d entry
cat > "${PKG_DIR}/${SOURCE_LIST_PATH}" <<EOF
deb [signed-by=/${KEYRING_PATH}] ${REPO_BASE_URL}/${DISTRIBUTION} ${CODENAME} main
EOF

# Copy keyring
cp "$KEYRING_OUTPUT" "${PKG_DIR}/${KEYRING_PATH}"

# Build the .deb package
DEB_FILE="${PACKAGE_NAME}_${VERSION}_all.deb"
dpkg-deb --build "$PKG_DIR" "${OUT_DIR}/${DEB_FILE}"

echo "✅ Created ${OUT_DIR}/${DEB_FILE}"
