#!/usr/bin/env bash

# Copyright (c) ZeroC, Inc.

# Shared GPG key import and validation logic for packaging scripts.
# Source this file in build scripts that need to sign packages or repositories.
#
# Required environment variables:
#   GPG_KEY     - The GPG private key in ASCII-armored format
#   GPG_KEY_ID  - The key ID to use for signing
#
# Usage:
#   source "$(dirname "${BASH_SOURCE[0]}")/../common/setup-gpg.sh"
#   setup_gpg

setup_gpg() {
    # Validate required environment variables
    : "${GPG_KEY:?GPG_KEY environment variable is not set}"
    : "${GPG_KEY_ID:?GPG_KEY_ID environment variable is not set}"

    # Import the GPG key
    echo "$GPG_KEY" | gpg --batch --import

    # Check that the key was successfully imported
    if ! gpg --list-secret-keys "$GPG_KEY_ID" > /dev/null 2>&1; then
        echo "Error: GPG key ID $GPG_KEY_ID was not imported successfully."
        exit 1
    fi

    echo "GPG key $GPG_KEY_ID imported successfully."
}
