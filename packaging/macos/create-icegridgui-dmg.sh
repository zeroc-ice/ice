#!/usr/bin/env bash
#
# Create a DMG from the IceGrid GUI macOS application bundle, and optionally
# sign and notarize it.
#
# Usage:
#   ./create-icegridgui-dmg.sh <app-path> <output-dir> <version> [--sign [--notarize]]
#
# Arguments:
#   app-path      Path to "IceGrid GUI.app"
#   output-dir    Directory where the DMG will be created
#   version       Version string for the DMG filename
#   --sign        Sign the DMG
#   --notarize    Also notarize and staple the DMG (requires APPLE_ID, APPLE_TEAM_ID,
#                 and APPLE_APP_SPECIFIC_PASSWORD environment variables)
#
# Environment:
#   SIGNING_IDENTITY              Code signing identity (default: "Developer ID Application: ZeroC, Inc. (U4TBVKNQ7F)")
#   APPLE_ID                      Apple ID for notarization (required with --notarize)
#   APPLE_TEAM_ID                 Apple Team ID for notarization (required with --notarize)
#   APPLE_APP_SPECIFIC_PASSWORD   App-specific password for notarization (required with --notarize)
#
# Example:
#   ./create-icegridgui-dmg.sh "output/IceGrid GUI.app" output 3.9.0
#   ./create-icegridgui-dmg.sh "output/IceGrid GUI.app" output 3.9.0 --sign --notarize
#

set -euo pipefail

APP_PATH="${1:?Usage: $0 <app-path> <output-dir> <version> [--sign [--notarize]]}"
OUTPUT_DIR="${2:?Usage: $0 <app-path> <output-dir> <version> [--sign [--notarize]]}"
VERSION="${3:?Usage: $0 <app-path> <output-dir> <version> [--sign [--notarize]]}"
SIGN=false
NOTARIZE=false
shift 3
while [ $# -gt 0 ]; do
    case "$1" in
        --sign) SIGN=true ;;
        --notarize) NOTARIZE=true ;;
        *) echo "Unknown option: $1" >&2; exit 1 ;;
    esac
    shift
done

SIGNING_IDENTITY="${SIGNING_IDENTITY:-Developer ID Application: ZeroC, Inc. (U4TBVKNQ7F)}"

APP_NAME="IceGrid GUI"
DMG_PATH="${OUTPUT_DIR}/IceGridGUI-${VERSION}.dmg"

echo "==> Creating DMG..."
mkdir -p "${OUTPUT_DIR}"
hdiutil create -volname "${APP_NAME}" \
    -srcfolder "${APP_PATH}" \
    -ov -format UDZO \
    "${DMG_PATH}"

if [ "${SIGN}" == "true" ]; then
    echo "==> Signing DMG..."
    codesign --force --timestamp --sign "${SIGNING_IDENTITY}" "${DMG_PATH}"

    echo "==> Verifying DMG signature..."
    codesign -vvv --strict "${DMG_PATH}"
fi

if [ "${NOTARIZE}" == "true" ]; then
    : "${APPLE_ID:?APPLE_ID must be set for notarization}"
    : "${APPLE_TEAM_ID:?APPLE_TEAM_ID must be set for notarization}"
    : "${APPLE_APP_SPECIFIC_PASSWORD:?APPLE_APP_SPECIFIC_PASSWORD must be set for notarization}"

    echo "==> Submitting DMG for notarization..."
    xcrun notarytool submit "${DMG_PATH}" \
        --apple-id "${APPLE_ID}" \
        --team-id "${APPLE_TEAM_ID}" \
        --password "${APPLE_APP_SPECIFIC_PASSWORD}" \
        --wait

    echo "==> Stapling notarization ticket..."
    xcrun stapler staple "${DMG_PATH}"

    echo "==> Verifying DMG with spctl..."
    spctl -a -t open --context context:primary-signature -vv "${DMG_PATH}"
fi

echo "==> Done. Created: ${DMG_PATH}"
