#!/bin/sh
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

if which swiftlint >/dev/null; then
    swiftlint --path "$1" --config "$2"
else
    echo "warning: SwiftLint not installed, download from https://github.com/realm/SwiftLint"
fi
