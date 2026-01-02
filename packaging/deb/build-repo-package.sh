#!/usr/bin/env bash

# This script builds a DEB package that installs the ZeroC Ice APT repository configuration.
#
# --distribution specifies the target distribution (e.g., debian12, debian13, or ubuntu24.04).
# --channel specifies the Ice version channel (e.g., 3.9, or 3.8).
# --quality specifies the release quality (e.g., stable, or nightly).
#
# The resulting package installs the generated list file into /etc/apt/sources.list.d/
# and the public GPG key into /usr/share/keyrings/zeroc-archive-keyring.gpg.
#
# The GPG key to include in the keyring must be provided via the GPG_KEY environment variable
# (in ASCII-armored format), and the corresponding key ID via GPG_KEY_ID.
#
# The build-deb-ice-repo-packages GitHub Actions workflow in this repository uses this script together
# with the ghcr.io/zeroc-ice/ice-deb-builder-<distribution>:<channel> Docker image to build the package.

set -euo pipefail

# Default values
DISTRIBUTION=""
CHANNEL=""
QUALITY=""
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

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
: "${GPG_KEY:?GPG_KEY environment variable is not set}"
: "${GPG_KEY_ID:?GPG_KEY_ID environment variable is not set}"

declare -A CODENAMES=(
    ["debian12"]="bookworm"
    ["debian13"]="trixie"
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
OUT_DIR="/workspace/build"
KEYRING_NAME="zeroc-archive-keyring.gpg"
if [[ "$QUALITY" == "stable" ]]; then
    PACKAGE_NAME="ice-repo-${CHANNEL}"
    REPO_BASE_URL="https://download.zeroc.com/ice/${CHANNEL}"
else
    PACKAGE_NAME="ice-repo-${CHANNEL}-${QUALITY}"
    REPO_BASE_URL="https://download.zeroc.com/ice/${QUALITY}/${CHANNEL}"
fi
KEYRING_PATH="usr/share/keyrings/${KEYRING_NAME}"
SOURCE_LIST_PATH="etc/apt/sources.list.d/${PACKAGE_NAME}.list"
PACKAGE_SPEC_DIR="${SCRIPT_DIR}/repo-packages/${PACKAGE_NAME}"
CONTROL_SRC="${PACKAGE_SPEC_DIR}/DEBIAN/control"
CHANGELOG_SRC="${PACKAGE_SPEC_DIR}/changelog.Debian"

if [[ ! -f "$CONTROL_SRC" ]]; then
  echo "Error: control file not found for ${PACKAGE_NAME} at ${CONTROL_SRC}" >&2
  exit 1
fi

if [[ ! -f "$CHANGELOG_SRC" ]]; then
  echo "Error: changelog file not found for ${PACKAGE_NAME} at ${CHANGELOG_SRC}" >&2
  exit 1
fi

# Determine version from changelog
VERSION="$(dpkg-parsechangelog --file "$CHANGELOG_SRC" --show-field Version)"
if [[ -z "$VERSION" ]]; then
  echo "Error: unable to determine Version from $CHANGELOG_SRC" >&2
  exit 1
fi

# Create keyring if needed
mkdir -p "${OUT_DIR}/keyrings"
KEYRING_OUTPUT="${OUT_DIR}/keyrings/${KEYRING_NAME}"
if [[ ! -f "$KEYRING_OUTPUT" ]]; then
  echo "Generating keyring..."
  gpg --export "$GPG_KEY_ID" | gpg --dearmor -o "$KEYRING_OUTPUT"
fi

# Create package layout
PKG_DIR="${OUT_DIR}/${PACKAGE_NAME}"
DOC_DIR="${PKG_DIR}/usr/share/doc/${PACKAGE_NAME}"
mkdir -p "${PKG_DIR}/DEBIAN"
mkdir -p "${PKG_DIR}/$(dirname "$SOURCE_LIST_PATH")"
mkdir -p "${PKG_DIR}/$(dirname "$KEYRING_PATH")"
mkdir -p "$DOC_DIR"

# Copy control and changelog from checked-in spec
cp "$CONTROL_SRC" "${PKG_DIR}/DEBIAN/control"
cp "$CHANGELOG_SRC" "${DOC_DIR}/changelog.Debian"
gzip -n --force "${DOC_DIR}/changelog.Debian"

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
