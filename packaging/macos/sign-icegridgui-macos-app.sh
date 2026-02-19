#!/usr/bin/env bash
#
# Sign and notarize the IceGrid GUI macOS application bundle.
#
# Usage:
#   ./sign-icegridgui-macos-app.sh <app-path> [--notarize]
#
# Arguments:
#   app-path      Path to "IceGrid GUI.app"
#   --notarize    Also notarize and staple (requires APPLE_ID, APPLE_TEAM_ID,
#                 and APPLE_APP_SPECIFIC_PASSWORD environment variables)
#
# Environment:
#   SIGNING_IDENTITY              Code signing identity (default: "Developer ID Application: ZeroC, Inc. (U4TBVKNQ7F)")
#   APPLE_ID                      Apple ID for notarization (required with --notarize)
#   APPLE_TEAM_ID                 Apple Team ID for notarization (required with --notarize)
#   APPLE_APP_SPECIFIC_PASSWORD   App-specific password for notarization (required with --notarize)
#
# Example:
#   ./sign-icegridgui-macos-app.sh "output/IceGrid GUI.app"
#   ./sign-icegridgui-macos-app.sh "output/IceGrid GUI.app" --notarize
#

set -euo pipefail

APP_PATH="$(cd "$(dirname "${1:?Usage: $0 <app-path> [--notarize]}")" && pwd)/$(basename "$1")"
NOTARIZE=false
if [ "${2:-}" == "--notarize" ]; then
    NOTARIZE=true
fi

SIGNING_IDENTITY="${SIGNING_IDENTITY:-Developer ID Application: ZeroC, Inc. (U4TBVKNQ7F)}"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ENTITLEMENTS="${SCRIPT_DIR}/entitlements.plist"

CODESIGN_ARGS=(--force --timestamp --options runtime --entitlements "${ENTITLEMENTS}" --sign "${SIGNING_IDENTITY}")

echo "==> Signing dylibs inside icegridgui.jar..."
# Extract, sign, and repackage dylibs inside the JAR
JAR_PATH="${APP_PATH}/Contents/app/icegridgui.jar"
if [ -f "${JAR_PATH}" ]; then
    WORKDIR=$(mktemp -d)
    trap 'rm -rf "$WORKDIR"' EXIT
    unzip "${JAR_PATH}" -d "${WORKDIR}/jar" > /dev/null
    find "${WORKDIR}/jar" -name "*.dylib" -exec codesign "${CODESIGN_ARGS[@]}" {} \;
    pushd "${WORKDIR}/jar" > /dev/null
    zip -r "${JAR_PATH}" ./* > /dev/null
    popd > /dev/null
fi

echo "==> Signing app bundle contents..."
find "${APP_PATH}/Contents/app" -type f -name "*.dylib" -exec codesign "${CODESIGN_ARGS[@]}" {} \;
find "${APP_PATH}/Contents/app" -type f -name "jspawnhelper" -exec codesign "${CODESIGN_ARGS[@]}" {} \;
find "${APP_PATH}/Contents/runtime" -type f -name "*.dylib" -exec codesign "${CODESIGN_ARGS[@]}" {} \;
find "${APP_PATH}/Contents/runtime" -type f -name "jspawnhelper" -exec codesign "${CODESIGN_ARGS[@]}" {} \;

echo "==> Signing runtime and app bundle..."
codesign "${CODESIGN_ARGS[@]}" "${APP_PATH}/Contents/runtime"
codesign "${CODESIGN_ARGS[@]}" "${APP_PATH}"

echo "==> Verifying signature..."
codesign -vvv --deep --strict "${APP_PATH}"
codesign -vvv --deep --strict "${APP_PATH}/Contents/runtime"

if [ "${NOTARIZE}" == "true" ]; then
    : "${APPLE_ID:?APPLE_ID must be set for notarization}"
    : "${APPLE_TEAM_ID:?APPLE_TEAM_ID must be set for notarization}"
    : "${APPLE_APP_SPECIFIC_PASSWORD:?APPLE_APP_SPECIFIC_PASSWORD must be set for notarization}"

    echo "==> Submitting app for notarization..."
    NOTARIZE_ZIP=$(mktemp /tmp/icegridgui-notarize-XXXXXX.zip)
    ditto -c -k --keepParent "${APP_PATH}" "${NOTARIZE_ZIP}"

    xcrun notarytool submit "${NOTARIZE_ZIP}" \
        --apple-id "${APPLE_ID}" \
        --team-id "${APPLE_TEAM_ID}" \
        --password "${APPLE_APP_SPECIFIC_PASSWORD}" \
        --wait

    rm -f "${NOTARIZE_ZIP}"

    echo "==> Stapling notarization ticket..."
    xcrun stapler staple "${APP_PATH}"

    echo "==> Verifying stapled app with Gatekeeper..."
    xcrun stapler validate "${APP_PATH}"
    spctl -a -t exec -vv "${APP_PATH}"
fi

echo "==> Done. Signed: ${APP_PATH}"
