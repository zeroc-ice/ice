#!/usr/bin/env bash

# Copyright (c) ZeroC, Inc.

# Shared Maven repository configuration for packaging scripts.
# Source this file in publish scripts that deploy to Maven repositories.
#
# Sets the REPO_ID and SOURCE_URL variables based on the release quality,
# generates ~/.m2/settings.xml with credentials, and imports the GPG signing key.
#
# Required environment variables:
#   CHANNEL         - The release channel (e.g., 3.9)
#   QUALITY         - Release quality (e.g., stable, nightly)
#   MAVEN_USERNAME  - Maven repository username
#   MAVEN_PASSWORD  - Maven repository password
#   GPG_KEY         - The GPG private key in ASCII-armored format
#   GPG_KEY_ID      - The key ID to use for signing
#
# Usage:
#   source "$(dirname "${BASH_SOURCE[0]}")/../common/setup-maven.sh"
#   setup_maven

setup_maven() {
    : "${QUALITY:?QUALITY environment variable is not set}"
    : "${MAVEN_USERNAME:?MAVEN_USERNAME environment variable is not set}"
    : "${MAVEN_PASSWORD:?MAVEN_PASSWORD environment variable is not set}"
    : "${GPG_KEY:?GPG_KEY environment variable is not set}"
    : "${GPG_KEY_ID:?GPG_KEY_ID environment variable is not set}"

    case "$QUALITY" in
      "stable")
        REPO_ID=ossrh
        SOURCE_URL="https://ossrh-staging-api.central.sonatype.com/service/local/staging/deploy/maven2/"
        ;;
      *)
        : "${CHANNEL:?CHANNEL environment variable is not set}"
        REPO_ID=maven-${CHANNEL}-${QUALITY}
        SOURCE_URL="https://download.zeroc.com/nexus/repository/maven-${CHANNEL}-${QUALITY}/"
        ;;
    esac

    # Generate Maven settings.xml
    mkdir -p ~/.m2
    cat > ~/.m2/settings.xml <<EOF
<?xml version="1.0" encoding="UTF-8"?>
<settings xmlns="http://maven.apache.org/SETTINGS/1.0.0"
          xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
          xsi:schemaLocation="http://maven.apache.org/SETTINGS/1.0.0 https://maven.apache.org/xsd/settings-1.0.0.xsd">
  <servers>
    <server>
      <id>${REPO_ID}</id>
      <username>${MAVEN_USERNAME}</username>
      <password>${MAVEN_PASSWORD}</password>
    </server>
  </servers>
</settings>
EOF

    # Import the signing GPG key.
    echo "$GPG_KEY" | gpg --batch --import

    echo "Maven repository configured: ${REPO_ID} (${SOURCE_URL})"
}
