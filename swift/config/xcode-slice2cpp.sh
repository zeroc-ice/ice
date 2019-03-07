#!/bin/sh

BASENAME=$(basename -- "$INPUT_FILE_PATH")
BASENAME="${BASENAME%.*}"
mkdir -p "$DERIVED_FILE_DIR/$1"
echo $SYMROOT/$CONFIGURATION/slice2cpp "$DERIVED_FILE_DIR/$1"
$SYMROOT/$CONFIGURATION/slice2cpp -I"$SRCROOT/../slice" --include-dir $1 --output-dir "$DERIVED_FILE_DIR/$1" "$INPUT_FILE_PATH"
mkdir -p "$SYMROOT/$PLATFORM_NAME/include/$1"
mv "$DERIVED_FILE_DIR/$1/$BASENAME.h" "$SYMROOT/$PLATFORM_NAME/include/$1/$BASENAME.h"
