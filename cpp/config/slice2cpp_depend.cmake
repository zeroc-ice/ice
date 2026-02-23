# Copyright (c) ZeroC, Inc.

# Helper script to generate a dependency file for Slice compilation.
# Invoked at build time via cmake -P with the following variables:
#   SLICE2CPP        - path to the slice2cpp executable
#   SLICE_FILE       - the .ice source file (absolute path)
#   SLICE_INCLUDE_DIR - path to Ice Slice directory (for -I flag)
#   OUTPUT_DIR       - directory where generated files are placed
#   DEPFILE          - output path for the generated .d file

execute_process(
    COMMAND "${SLICE2CPP}" "-I${SLICE_INCLUDE_DIR}" --depend "${SLICE_FILE}"
    OUTPUT_VARIABLE depend_output
    ERROR_VARIABLE depend_error
    RESULT_VARIABLE result
)

if(NOT result EQUAL 0)
    message(FATAL_ERROR "slice2cpp --depend failed for ${SLICE_FILE}: ${depend_error}")
endif()

# The --depend output has a bare header filename as the target, e.g.:
#   Locator.h: \
#    /path/to/Locator.ice \
#    /path/to/Identity.ice
#
# Replace the target with the full output path so it matches the OUTPUT of add_custom_command.
# We match ": \" (the depfile target separator) instead of just ":" to avoid matching the drive
# letter in Windows paths like C:/... (where the colon is followed by / or \, never by " \").
string(REGEX REPLACE "^([^:]+)(: \\\\)" "${OUTPUT_DIR}/\\1\\2" depend_output "${depend_output}")

file(WRITE "${DEPFILE}" "${depend_output}")
