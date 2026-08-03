# Copyright (c) ZeroC, Inc.

# Compiles the Slice (.ice) files in a target's sources and adds the generated C++ to the target.
# Only sees the sources present when it is called.
#
#   slice2cpp_generate(<target>
#     [INCLUDE_DIRS <dir>...]     # extra -I directories
#     [OPTIONS <option>...]       # extra slice2cpp options, e.g. -DFOO
#     [HEADER_OUTPUT_DIR <dir>]   # put the headers here instead of with the sources
#     [INCLUDE_DIR <dir>]         # slice2cpp --include-dir
#     [INCLUDE_SCOPE <scope>]     # PRIVATE (default) or PUBLIC
#   )
#
# Example:
#   add_executable(a_target source1.cpp source2.ice source3.ice)
#   slice2cpp_generate(a_target)
#
# A library whose Slice includes files from a shared directory, publishing its generated headers to
# its own consumers under a "Demo/" prefix:
#
#   add_library(demo Greeter.ice)
#   slice2cpp_generate(demo
#     INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../slice
#     OPTIONS -DENABLE_EXTRAS
#     HEADER_OUTPUT_DIR ${CMAKE_BINARY_DIR}/include
#     INCLUDE_DIR Demo
#     INCLUDE_SCOPE PUBLIC)
#
# Only the .ice files in the target's own sources are compiled. INCLUDE_DIRS just resolves the
# includes, so a Slice file included from a shared directory needs a target that compiles it, and
# `demo` links that target to pick up its headers.
#
# Generated files mirror the layout of the .ice files under the include directory that reaches them,
# so the #include directives slice2cpp emits resolve. Headers land in HEADER_OUTPUT_DIR/INCLUDE_DIR
# when either is given, and that root goes on the target's include path.

# The function records the policies in force when it is defined; pin them so a consumer with an older
# policy baseline gets the same behavior. include() scopes this to the current file.
cmake_policy(VERSION 3.21)

function(slice2cpp_generate target)
  cmake_parse_arguments(PARSE_ARGV 1 arg "" "INCLUDE_SCOPE;HEADER_OUTPUT_DIR;INCLUDE_DIR" "INCLUDE_DIRS;OPTIONS")

  if(NOT TARGET ${target})
    message(FATAL_ERROR "slice2cpp_generate: '${target}' is not a target.")
  endif()

  if(arg_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "slice2cpp_generate: unexpected arguments: ${arg_UNPARSED_ARGUMENTS}")
  endif()

  if(NOT arg_INCLUDE_SCOPE)
    set(arg_INCLUDE_SCOPE PRIVATE)
  elseif(NOT arg_INCLUDE_SCOPE MATCHES "^(PRIVATE|PUBLIC)$")
    # Not INTERFACE: the target compiles the generated sources, so it needs the directory itself.
    message(FATAL_ERROR "slice2cpp_generate: INCLUDE_SCOPE must be PRIVATE or PUBLIC.")
  endif()

  get_target_property(sources ${target} SOURCES)
  if(NOT sources)
    return()
  endif()

  set(output_dir ${CMAKE_CURRENT_BINARY_DIR}/generated/${target})
  file(MAKE_DIRECTORY ${output_dir})

  # Root the headers are written under. INCLUDE_DIR names a directory created beneath it, rather than
  # a suffix the caller has to repeat in HEADER_OUTPUT_DIR, so the prefix each generated source uses
  # always resolves against a directory that is on the include path.
  if(arg_HEADER_OUTPUT_DIR)
    get_filename_component(header_root "${arg_HEADER_OUTPUT_DIR}" ABSOLUTE)
  else()
    set(header_root ${output_dir})
  endif()

  set(header_output_dir ${header_root})
  if(arg_INCLUDE_DIR)
    set(header_output_dir ${header_root}/${arg_INCLUDE_DIR})
  endif()
  file(MAKE_DIRECTORY ${header_output_dir})

  set(generated_include_dirs ${output_dir})
  if(NOT header_root STREQUAL output_dir)
    list(APPEND generated_include_dirs ${header_root})
  endif()

  # BUILD_INTERFACE keeps these build paths out of a PUBLIC consumer's exported usage requirements.
  foreach(dir IN LISTS generated_include_dirs)
    target_include_directories(${target} ${arg_INCLUDE_SCOPE} $<BUILD_INTERFACE:${dir}>)
  endforeach()

  # Ice first, so #include <Ice/...> keeps working.
  set(include_dirs ${Ice_SLICE_DIR} ${arg_INCLUDE_DIRS})
  set(include_options "")
  foreach(dir IN LISTS include_dirs)
    list(APPEND include_options "-I${dir}")
  endforeach()

  # $<SEMICOLON> survives argument splitting, so the -P script receives one real list.
  list(JOIN include_dirs "$<SEMICOLON>" include_dirs_arg)
  list(JOIN arg_OPTIONS "$<SEMICOLON>" options_arg)

  # Roots the layout can mirror. The source directory counts: "sub/Foo.ice" resolves relative to the
  # including file.
  set(mirror_roots "")
  foreach(dir IN LISTS include_dirs CMAKE_CURRENT_SOURCE_DIR)
    get_filename_component(dir "${dir}" ABSOLUTE)
    list(APPEND mirror_roots "${dir}")
  endforeach()

  foreach(file IN LISTS sources)
    if(file MATCHES "\\.ice$")

      get_filename_component(slice_file_name ${file} NAME_WE)
      get_filename_component(slice_file_path ${file} ABSOLUTE)
      get_filename_component(slice_file_dir ${slice_file_path} DIRECTORY)

      # Mirror against the deepest include root holding this file: that is the relative path
      # slice2cpp used, and generating flat would collide same-named .ice files.
      set(slice_file_subdir "")
      set(best_length -1)
      foreach(root IN LISTS mirror_roots)
        file(RELATIVE_PATH candidate "${root}" "${slice_file_dir}")
        if(NOT candidate MATCHES "^\\.\\.$|^\\.\\.[\\\\/]" AND NOT IS_ABSOLUTE "${candidate}")
          string(LENGTH "${candidate}" candidate_length)
          if(best_length LESS 0 OR candidate_length LESS best_length)
            set(slice_file_subdir "${candidate}")
            set(best_length ${candidate_length})
          endif()
        endif()
      endforeach()

      if(slice_file_subdir STREQUAL "" OR slice_file_subdir STREQUAL ".")
        set(file_output_dir ${output_dir})
        set(file_header_dir ${header_output_dir})
      else()
        set(file_output_dir ${output_dir}/${slice_file_subdir})
        set(file_header_dir ${header_output_dir}/${slice_file_subdir})
      endif()
      get_filename_component(file_output_dir ${file_output_dir} ABSOLUTE)
      get_filename_component(file_header_dir ${file_header_dir} ABSOLUTE)
      file(MAKE_DIRECTORY ${file_output_dir})
      file(MAKE_DIRECTORY ${file_header_dir})

      file(RELATIVE_PATH output_dir_relative ${CMAKE_CURRENT_LIST_DIR} ${file_output_dir})
      file(RELATIVE_PATH header_dir_relative ${CMAKE_CURRENT_LIST_DIR} ${file_header_dir})

      set(header_file ${file_header_dir}/${slice_file_name}.h)
      set(source_file ${file_output_dir}/${slice_file_name}.cpp)

      # Prefix for the #include each generated source uses to reach its own header. It has to follow
      # the mirrored layout, since only the roots above are on the include path. A separate header
      # directory means the source can no longer find the header beside itself, so a mirrored file
      # needs the prefix even without INCLUDE_DIR.
      set(include_prefix "${arg_INCLUDE_DIR}")
      if(NOT slice_file_subdir STREQUAL "" AND NOT slice_file_subdir STREQUAL ".")
        if(include_prefix)
          set(include_prefix "${include_prefix}/${slice_file_subdir}")
        elseif(NOT header_root STREQUAL output_dir)
          set(include_prefix "${slice_file_subdir}")
        endif()
      endif()

      set(include_dir_options "")
      if(include_prefix)
        set(include_dir_options --include-dir ${include_prefix})
      endif()

      # Header first: Ninja requires the depfile target to match the first output.
      set(output_files ${header_file} ${source_file})
      set(depfile ${file_output_dir}/${slice_file_name}.d)

      # slice2cpp has no separate header option, so move it afterwards as the MSBuild task does.
      # Copy then remove, since rename is limited to a single volume.
      set(move_header_commands "")
      if(NOT file_header_dir STREQUAL file_output_dir)
        set(move_header_commands
          COMMAND ${CMAKE_COMMAND} -E copy ${file_output_dir}/${slice_file_name}.h ${header_file}
          COMMAND ${CMAKE_COMMAND} -E rm -f ${file_output_dir}/${slice_file_name}.h)
      endif()

      add_custom_command(
        OUTPUT ${output_files}
        COMMAND ${CMAKE_COMMAND}
          -DSLICE2CPP=$<TARGET_FILE:Ice::slice2cpp>
          -DSLICE_FILE=${slice_file_path}
          -DSLICE_INCLUDE_DIRS=${include_dirs_arg}
          -DSLICE_OPTIONS=${options_arg}
          -DHEADER_DIR=${file_header_dir}
          -DDEPFILE=${depfile}
          -P ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/slice2cpp_depend.cmake
        COMMAND $<TARGET_FILE:Ice::slice2cpp> ${include_options} ${include_dir_options} ${arg_OPTIONS}
          ${slice_file_path} --output-dir ${file_output_dir}
        ${move_header_commands}
        DEPENDS ${slice_file_path} $<TARGET_FILE:Ice::slice2cpp>
        DEPFILE ${depfile}
        VERBATIM
        COMMENT "Compiling Slice ${file} -> ${output_dir_relative}/${slice_file_name}.cpp ${header_dir_relative}/${slice_file_name}.h"
      )

      target_sources(${target} PRIVATE ${output_files})

    endif()
  endforeach()
endfunction()
