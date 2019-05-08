#!/bin/sh
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

if [ -f "$ICE_HOME/cpp/bin/slice2swift" ]; then
    SLICE2SWIFT="$ICE_HOME/cpp/bin/slice2swift"
elif [ -f "$SRCROOT/../cpp/bin/slice2swift" ]; then
    SLICE2SWIFT="$SRCROOT/../cpp/bin/slice2swift"
elif [ -f /usr/local/bin/slice2swift ]; then
    SLICE2SWIFT=/usr/local/bin/slice2swift
else
    echo "Failed to locate slice2swift compiler"
    exit 1
fi

if [ -z "$1" ]; then
    $SLICE2SWIFT -I"$SRCROOT/../slice" -I"$INPUT_FILE_DIR" --output-dir "$DERIVED_FILE_DIR" "$INPUT_FILE_PATH"
else
    BASENAME=$(basename -- "$INPUT_FILE_PATH")
    BASENAME="${BASENAME%.*}"
    mkdir -p "$DERIVED_FILE_DIR/$1"
    $SLICE2SWIFT -I"$SRCROOT/../slice" -I"$INPUT_FILE_DIR" --output-dir "$DERIVED_FILE_DIR/$1" "$INPUT_FILE_PATH"
    mv "$DERIVED_FILE_DIR/$1/$BASENAME.swift" "$DERIVED_FILE_DIR/$1_$BASENAME.swift"
fi
