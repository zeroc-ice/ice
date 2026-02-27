#!/usr/bin/env bash
#
# Build the IceGrid GUI macOS application bundle (.app) from icegridgui.jar.
#
# Usage:
#   ./build-icegridgui-macos-app.sh <jar-path> <output-dir>
#
# Arguments:
#   jar-path    Path to icegridgui.jar
#   output-dir  Directory where "IceGrid GUI.app" will be created
#
# Environment:
#   JAVA_HOME   Must point to a Java 17+ installation
#
# Example:
#   ./build-icegridgui-macos-app.sh java/lib/icegridgui.jar output
#

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"

JAR_PATH="${1:?Usage: $0 <jar-path> <output-dir>}"
DEST_DIR="${2:?Usage: $0 <jar-path> <output-dir>}"

# shellcheck source=../../config/version.env
source "${REPO_ROOT}/config/version.env"
APP_VERSION="${BASE_VERSION}"

JAVA_HOME="${JAVA_HOME:?JAVA_HOME must be set to a Java 17+ installation}"
JPACKAGE="${JAVA_HOME}/bin/jpackage"
JLINK="${JAVA_HOME}/bin/jlink"

APP_NAME="IceGrid GUI"
ICON_PATH="${REPO_ROOT}/java/src/IceGridGUI/src/main/resources/icons/icegrid.icns"

WORKDIR=$(mktemp -d)
trap 'rm -rf "$WORKDIR"' EXIT

echo "==> Preparing input directory..."
INPUT_DIR="${WORKDIR}/input"
mkdir -p "${INPUT_DIR}"
cp "${JAR_PATH}" "${INPUT_DIR}/icegridgui.jar"

echo "==> Building Java JRE with jlink..."
# Module list determined by: jdeps --list-deps icegridgui.jar
DEPS="java.base,java.datatransfer,java.desktop,java.logging,java.naming,java.prefs,java.xml,jdk.unsupported,jdk.unsupported.desktop"
"${JLINK}" \
    --add-modules "${DEPS}" \
    --strip-debug \
    --no-header-files \
    --no-man-pages \
    --strip-native-commands \
    --output "${WORKDIR}/jre"

echo "==> Creating ${APP_NAME}.app (version ${APP_VERSION})..."
mkdir -p "${DEST_DIR}"
"${JPACKAGE}" --type app-image \
    --name "${APP_NAME}" \
    --app-version "${APP_VERSION}" \
    --main-class com.zeroc.IceGridGUI/Main \
    --main-jar "icegridgui.jar" \
    --icon "${ICON_PATH}" \
    --copyright "Copyright © ZeroC, Inc. All rights reserved." \
    --vendor "ZeroC, Inc." \
    --input "${INPUT_DIR}" \
    --dest "${DEST_DIR}" \
    --runtime-image "${WORKDIR}/jre"

echo "==> Created ${DEST_DIR}/${APP_NAME}.app"
