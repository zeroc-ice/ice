#!/usr/bin/env bash

# Copyright (c) ZeroC, Inc.

# Map distribution identifiers to Debian/Ubuntu codenames.
# Source this file in scripts that need codename mapping.

declare -A CODENAMES=(
    ["debian12"]="bookworm"
    ["debian13"]="trixie"
    ["ubuntu24.04"]="noble"
)
