#!/bin/sh
TOP_LEVEL=$(git rev-parse --show-toplevel)

if [ -z "$1" ]; then
    ${TOP_LEVEL}/cpp/bin/slice2swift -I"${TOP_LEVEL}/Slice" -I"${INPUT_FILE_DIR}" --output-dir "$DERIVED_FILE_DIR" "$INPUT_FILE_PATH"
else
    BASENAME=$(basename -- "$INPUT_FILE_PATH")
    BASENAME="${BASENAME%.*}"
    mkdir -p "$DERIVED_FILE_DIR/$1"
    ${TOP_LEVEL}/cpp/bin/slice2swift -I"${TOP_LEVEL}/Slice" -I"${INPUT_FILE_DIR}" --output-dir "$DERIVED_FILE_DIR/$1" "$INPUT_FILE_PATH"
    mv "$DERIVED_FILE_DIR/$1/$BASENAME.swift" "$DERIVED_FILE_DIR/$1_$BASENAME.swift"
fi
