# Copyright (c) ZeroC, Inc.

# Helper script to generate a dependency file for Slice compilation.
# Invoked at build time via cmake -P with the following variables:
#   SLICE2CPP          - path to the slice2cpp executable
#   SLICE_FILE         - the .ice source file (absolute path)
#   SLICE_INCLUDE_DIRS - list of Slice include directories (one -I flag each)
#   SLICE_OPTIONS      - extra slice2cpp options; -D/-U change which includes --depend sees
#   HEADER_DIR         - directory the generated header ends up in
#   EXPECTED_HEADER    - header name declared to CMake, checked against what slice2cpp will write
#   DEPFILE            - output path for the generated .d file

# Escapes a path the way compiler depfiles do, so it can be compared or joined with the paths
# slice2cpp writes: '$'->'$$', '#'->'\#', whitespace get a preceding backslash,
# and a run of backslashes immediately before whitespace is doubled.
function(escape_depfile_path out_var path)
    string(REPLACE "$" "$$" path "${path}")
    string(REPLACE "#" "\\#" path "${path}")
    string(REGEX REPLACE "(\\\\*)([ \t])" "\\1\\1\\\\\\2" path "${path}")
    set(${out_var} "${path}" PARENT_SCOPE)
endfunction()

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
if(NOT depend_output MATCHES "^([^:]+): \\\\")
    message(FATAL_ERROR "slice2cpp --depend produced unexpected output for ${SLICE_FILE}:\n${depend_output}")
endif()
set(actual_header "${CMAKE_MATCH_1}")

# --depend names the header slice2cpp will actually write, cpp:header-ext metadata included, in
# depfile-escaped form. Report a mismatch here rather than rerun forever a command whose declared
# output never appears.
escape_depfile_path(expected_header_escaped "${EXPECTED_HEADER}")
if(NOT actual_header STREQUAL expected_header_escaped)
    message(FATAL_ERROR
        "slice2cpp generates '${actual_header}' for ${SLICE_FILE}, but slice2cpp_generate declared "
        "'${EXPECTED_HEADER}'. Set the extension with OPTIONS --header-ext; cpp:header-ext metadata "
        "that contradicts it is not supported.")
endif()

# The prefix is a path slice2cpp never sees, so escape it here the way compiler depfiles escape
# paths. The paths slice2cpp itself writes arrive already escaped.
escape_depfile_path(header_dir_escaped "${HEADER_DIR}")

string(LENGTH "${actual_header}" target_length)
string(SUBSTRING "${depend_output}" ${target_length} -1 depend_tail)
file(WRITE "${DEPFILE}" "${header_dir_escaped}/${actual_header}${depend_tail}")
