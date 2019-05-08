#!/bin/sh
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

if [ -f "$ICE_HOME/cpp/bin/slice2cpp" ]; then
    SLICE2CPP="$ICE_HOME/cpp/bin/slice2cpp"
elif [ -f "$SRCROOT/../cpp/bin/slice2cpp" ]; then
    SLICE2CPP="$SRCROOT/../cpp/bin/slice2cpp"
elif [ -f /usr/local/bin/slice2cpp ]; then
    SLICE2CPP=/usr/local/bin/slice2cpp
else
    echo "Failed to locate slice2cpp compiler"
    exit 1
fi

BASENAME=$(basename -- "$INPUT_FILE_PATH")
BASENAME="${BASENAME%.*}"
mkdir -p "$DERIVED_FILE_DIR/$1"
$SLICE2CPP -I"$SRCROOT/../slice" -D ICE_SWIFT --include-dir $1 --output-dir "$DERIVED_FILE_DIR/$1" "$INPUT_FILE_PATH"
mkdir -p "$SYMROOT/$PLATFORM_NAME/include/$1"
mv "$DERIVED_FILE_DIR/$1/$BASENAME.h" "$SYMROOT/$PLATFORM_NAME/include/$1/$BASENAME.h"
