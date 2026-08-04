# Copyright (c) ZeroC, Inc.

# Helper script to generate a dependency file for Slice compilation.
# Invoked at build time via cmake -P with the following variables:
#   SLICE2CPP          - path to the slice2cpp executable
#   SLICE_FILE         - the .ice source file (absolute path)
#   SLICE_INCLUDE_DIRS - list of Slice include directories (one -I flag each)
#   SLICE_OPTIONS      - extra slice2cpp options; -D/-U change which includes the preprocessor sees,
#                        so --depend needs them to compute the same dependencies as the compile
#   HEADER_DIR         - directory the generated header ends up in
#   EXPECTED_HEADER    - header name declared to CMake, checked against what slice2cpp will write
#   DEPFILE            - output path for the generated .d file

set(include_options "")
foreach(dir IN LISTS SLICE_INCLUDE_DIRS)
    list(APPEND include_options "-I${dir}")
endforeach()

execute_process(
    COMMAND "${SLICE2CPP}" ${include_options} ${SLICE_OPTIONS} --depend "${SLICE_FILE}"
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
# Rewrite the target to the full header path, which Ninja requires it to match the first OUTPUT.
# Match ": \" rather than ":" so Windows drive letters like C:/... are not mistaken for the
# separator (there the colon is followed by / or \, never by " \").
if(NOT depend_output MATCHES "^([^:]+)(: \\\\)")
    message(FATAL_ERROR "slice2cpp --depend produced unexpected output for ${SLICE_FILE}:\n${depend_output}")
endif()

# --depend names the header slice2cpp will actually write, which honors cpp:header-ext metadata over
# the --header-ext option. CMake committed to a name at configure time; report a mismatch here rather
# than rerun a command forever whose declared output is never created.
if(NOT CMAKE_MATCH_1 STREQUAL EXPECTED_HEADER)
    message(FATAL_ERROR
        "slice2cpp generates '${CMAKE_MATCH_1}' for ${SLICE_FILE}, but slice2cpp_generate declared "
        "'${EXPECTED_HEADER}'. Set the extension with OPTIONS --header-ext; cpp:header-ext metadata "
        "that contradicts it is not supported.")
endif()

string(REGEX REPLACE "^([^:]+)(: \\\\)" "${HEADER_DIR}/\\1\\2" depend_output "${depend_output}")

file(WRITE "${DEPFILE}" "${depend_output}")
