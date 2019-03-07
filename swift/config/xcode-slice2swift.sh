#!/bin/sh

if [ -z "$1" ]; then
    $SYMROOT/$CONFIGURATION/slice2swift -I"$SRCROOT/../slice" -I"$INPUT_FILE_DIR" --output-dir "$DERIVED_FILE_DIR" "$INPUT_FILE_PATH"
else
    BASENAME=$(basename -- "$INPUT_FILE_PATH")
    BASENAME="${BASENAME%.*}"
    mkdir -p "$DERIVED_FILE_DIR/$1"
    $SYMROOT/$CONFIGURATION/slice2swift -I"$SRCROOT/../slice" -I"$INPUT_FILE_DIR" --output-dir "$DERIVED_FILE_DIR/$1" "$INPUT_FILE_PATH"
    mv "$DERIVED_FILE_DIR/$1/$BASENAME.swift" "$DERIVED_FILE_DIR/$1_$BASENAME.swift"
fi
