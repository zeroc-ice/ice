#!/bin/sh
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

SLICE2CPP=$SRCROOT/../cpp/bin/slice2cpp

BASENAME=$(basename -- "$INPUT_FILE_PATH")
BASENAME="${BASENAME%.*}"
mkdir -p "$DERIVED_FILE_DIR/$1"
$SLICE2CPP -I"$SRCROOT/../slice" -D ICE_SWIFT --include-dir $1 --output-dir "$DERIVED_FILE_DIR/$1" "$INPUT_FILE_PATH"
mkdir -p "$SYMROOT/$PLATFORM_NAME/include/$1"
mv "$DERIVED_FILE_DIR/$1/$BASENAME.h" "$SYMROOT/$PLATFORM_NAME/include/$1/$BASENAME.h"
