#!/bin/sh

if [ -f /usr/local/opt/ice/bin/slice2swift ]; then
    SLICE2SWIFT=/usr/local/bin/slice2swift
else
    SLICE2SWIFT=$SRCROOT/../cpp/bin/slice2swift
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
