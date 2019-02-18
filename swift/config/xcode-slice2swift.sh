#!/bin/sh
TOP_LEVEL=$(git rev-parse --show-toplevel)
${TOP_LEVEL}/cpp/bin/slice2swift -I${TOP_LEVEL}/Slice -I${INPUT_FILE_DIR} --output-dir $DERIVED_FILE_DIR $INPUT_FILE_PATH
