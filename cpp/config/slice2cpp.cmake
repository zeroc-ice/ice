# Copyright (c) ZeroC, Inc.

# Compiles the Slice (.ice) files in a target's sources and adds the generated C++ to the target.
# Only sees the sources present when it is called.
#
#   slice2cpp_generate(<target>
#     [INCLUDE_DIRS <dir>...]     # extra -I directories
#     [OPTIONS <option>...]       # extra slice2cpp options, e.g. -DFOO or --header-ext hpp
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
# Generated files mirror the layout of the .ice files under the INCLUDE_DIRS directory that reaches
# them, so the #include directives slice2cpp emits resolve; a file under no such directory generates
# flat, and two flat files sharing a name is reported at configure time. Headers land under
# HEADER_OUTPUT_DIR/INCLUDE_DIR when given, and both that directory and HEADER_OUTPUT_DIR go on the
# target's include path.
#
# A PUBLIC INCLUDE_SCOPE covers the build tree only; installing or exporting the target additionally
# needs the caller to install the generated headers and add an INSTALL_INTERFACE directory.
#
# The generated file names are declared to CMake at configure time: --header-ext and --source-ext
# in OPTIONS are honored, options that relocate the outputs or suppress generation are rejected, and
# cpp:header-ext metadata that contradicts the declared name is reported when the file compiles.

# Checked up front with a readable message: this file is loaded by every consumer of the Ice
# package, not only the ones that generate Slice.
if(CMAKE_VERSION VERSION_LESS 3.21)
  message(FATAL_ERROR "The Ice CMake package requires CMake 3.21 or later.")
endif()

# The function records the policies in force when it is defined; pin them so a consumer with an older
# policy baseline gets the same behavior. include() scopes this to the current file.
cmake_policy(VERSION 3.21)

function(slice2cpp_generate target)
  cmake_parse_arguments(PARSE_ARGV 1 arg "" "INCLUDE_SCOPE;HEADER_OUTPUT_DIR;INCLUDE_DIR" "INCLUDE_DIRS;OPTIONS")

  if(NOT TARGET ${target})
    message(FATAL_ERROR "slice2cpp_generate: '${target}' is not a target.")
  endif()

  get_target_property(aliased ${target} ALIASED_TARGET)
  if(aliased)
    message(FATAL_ERROR "slice2cpp_generate: '${target}' is an alias of '${aliased}'; pass that target.")
  endif()

  # target_sources(PRIVATE) below needs a target that compiles; anything else fails later, from
  # inside this function, with a far less direct message.
  get_target_property(target_type ${target} TYPE)
  if(NOT target_type MATCHES "^(EXECUTABLE|STATIC_LIBRARY|SHARED_LIBRARY|MODULE_LIBRARY|OBJECT_LIBRARY)$")
    message(FATAL_ERROR "slice2cpp_generate: '${target}' is a ${target_type}, which cannot compile Slice.")
  endif()

  if(arg_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "slice2cpp_generate: unexpected arguments: ${arg_UNPARSED_ARGUMENTS}")
  endif()

  if(arg_KEYWORDS_MISSING_VALUES)
    message(FATAL_ERROR "slice2cpp_generate: missing value for: ${arg_KEYWORDS_MISSING_VALUES}")
  endif()

  if(NOT arg_INCLUDE_SCOPE)
    set(arg_INCLUDE_SCOPE PRIVATE)
  elseif(NOT arg_INCLUDE_SCOPE MATCHES "^(PRIVATE|PUBLIC)$")
    # Not INTERFACE: the target compiles the generated sources, so it needs the directory itself.
    message(FATAL_ERROR "slice2cpp_generate: INCLUDE_SCOPE must be PRIVATE or PUBLIC.")
  endif()

  # INCLUDE_DIR names a directory created under the header root and the prefix the generated sources
  # include it by, so it has to stay a relative path that does not climb out of that root.
  if(arg_INCLUDE_DIR AND (IS_ABSOLUTE "${arg_INCLUDE_DIR}" OR arg_INCLUDE_DIR MATCHES "(^|[\\/])\.\.([\\/]|$)"))
    message(FATAL_ERROR
      "slice2cpp_generate: INCLUDE_DIR must be a relative path without '..', got '${arg_INCLUDE_DIR}'.")
  endif()

  # The outputs are declared to CMake below, so an option that changes what slice2cpp writes has to
  # be reflected there or the declared files are never written - which Ninja then retries on every
  # build without saying why. --header-ext and --source-ext are read and honored, the way the MSBuild
  # task treats them; options that relocate the outputs or replace generation are rejected.
  set(header_ext "h")
  set(source_ext "cpp")
  list(LENGTH arg_OPTIONS options_count)
  set(option_index 0)
  while(option_index LESS options_count)
    list(GET arg_OPTIONS ${option_index} option)
    set(option_value "")

    if(option MATCHES "^--(header|source)-ext=(.+)$")
      set(option_kind "${CMAKE_MATCH_1}")
      set(option_value "${CMAKE_MATCH_2}")
    elseif(option MATCHES "^--(header|source)-ext$")
      set(option_kind "${CMAKE_MATCH_1}")
      math(EXPR option_index "${option_index} + 1")
      if(NOT option_index LESS options_count)
        message(FATAL_ERROR "slice2cpp_generate: '${option}' is missing its argument.")
      endif()
      list(GET arg_OPTIONS ${option_index} option_value)
    endif()

    if(option_value)
      if(NOT option_value MATCHES "^[A-Za-z0-9]+$")
        message(FATAL_ERROR "slice2cpp_generate: '${option_value}' is not a valid ${option_kind} extension.")
      endif()
      set(${option_kind}_ext "${option_value}")
    elseif(option MATCHES "^(-I|--output-dir|--include-dir|--depend)")
      message(FATAL_ERROR
        "slice2cpp_generate: '${option}' is managed by slice2cpp_generate and cannot be passed in "
        "OPTIONS; use INCLUDE_DIRS, HEADER_OUTPUT_DIR or INCLUDE_DIR instead.")
    elseif(option MATCHES "^(-h|--help|-v|--version|--validate)$" OR option MATCHES "\\.ice$")
      message(FATAL_ERROR
        "slice2cpp_generate: OPTIONS takes slice2cpp options that affect code generation; "
        "'${option}' suppresses generation or names an input file.")
    endif()

    math(EXPR option_index "${option_index} + 1")
  endwhile()

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

  # header_root resolves the INCLUDE_DIR-prefixed include each generated source uses for its own
  # header; header_output_dir resolves the includes slice2cpp emits for other generated headers,
  # which carry no such prefix. BUILD_INTERFACE keeps these build paths out of a PUBLIC consumer's
  # exported usage requirements.
  set(generated_include_dirs ${output_dir})
  foreach(dir IN ITEMS ${header_root} ${header_output_dir})
    if(NOT dir IN_LIST generated_include_dirs)
      list(APPEND generated_include_dirs ${dir})
    endif()
  endforeach()
  foreach(dir IN LISTS generated_include_dirs)
    target_include_directories(${target} ${arg_INCLUDE_SCOPE} $<BUILD_INTERFACE:${dir}>)
  endforeach()

  # Absolute once, here: this list is both the -I set slice2cpp receives and the roots the generated
  # layout mirrors. A relative entry would otherwise reach slice2cpp unchanged and resolve against
  # the build directory, while the mirror resolved it against the source directory.
  # Ice first, so #include <Ice/...> keeps working.
  set(include_dirs "")
  foreach(dir IN LISTS Ice_SLICE_DIR arg_INCLUDE_DIRS)
    get_filename_component(dir "${dir}" ABSOLUTE)
    list(APPEND include_dirs "${dir}")
  endforeach()
  set(include_options "")
  foreach(dir IN LISTS include_dirs)
    list(APPEND include_options "-I${dir}")
  endforeach()

  # $<SEMICOLON> survives argument splitting, so the -P script receives one real list.
  list(JOIN include_dirs "$<SEMICOLON>" include_dirs_arg)
  list(JOIN arg_OPTIONS "$<SEMICOLON>" options_arg)

  foreach(file IN LISTS sources)
    if(file MATCHES "\\.ice$")

      get_filename_component(slice_file_path ${file} ABSOLUTE)
      get_filename_component(slice_file_dir ${slice_file_path} DIRECTORY)

      # NAME_WLE, not NAME_WE: slice2cpp strips only the final extension, so Foo.v1.ice generates
      # Foo.v1.h, and NAME_WE would declare an output named Foo.h that is never written.
      get_filename_component(slice_file_name ${slice_file_path} NAME_WLE)

      # Subdirectory this file mirrors into; "." is the root. Only an INCLUDE_DIRS directory can
      # drive the layout, because the #include slice2cpp emits for an included Slice file is derived
      # from the -I list alone. Match its rule: the root giving the shortest relative path wins, and
      # an exact root scores zero so it beats a broader root giving an equally short name. A file
      # under no -I directory generates flat, as it always has.
      set(slice_file_subdir ".")
      set(best_length -1)
      foreach(root IN LISTS include_dirs)
        file(RELATIVE_PATH candidate "${root}" "${slice_file_dir}")
        if(candidate STREQUAL "" OR candidate STREQUAL ".")
          set(candidate ".")
          set(candidate_length 0)
        else()
          string(LENGTH "${candidate}" candidate_length)
        endif()
        if(NOT candidate MATCHES "^\\.\\.$|^\\.\\.[\\\\/]" AND NOT IS_ABSOLUTE "${candidate}")
          if(best_length LESS 0 OR candidate_length LESS best_length)
            set(slice_file_subdir "${candidate}")
            set(best_length ${candidate_length})
          endif()
        endif()
      endforeach()

      if(slice_file_subdir STREQUAL ".")
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

      set(header_file ${file_header_dir}/${slice_file_name}.${header_ext})
      set(source_file ${file_output_dir}/${slice_file_name}.${source_ext})

      # Two commands writing one header races under Makefile generators and is a hard error under
      # Ninja, and neither message names the targets or Slice files involved. Claim each header
      # globally and report what actually clashed; two same-named flat .ice files land here too.
      # Hash the path rather than MAKE_C_IDENTIFIER, which folds /a/b and /a_b together; store one
      # preformatted string so a ';' in a path cannot split it into a list.
      string(SHA256 header_key "${header_file}")
      set(owner_property "_slice2cpp_generate_owner_${header_key}")
      get_property(owner GLOBAL PROPERTY ${owner_property})
      if(owner)
        if(owner STREQUAL "${target}|${slice_file_path}")
          # The same .ice listed twice for this target; declaring the command again would give the
          # outputs two rules.
          continue()
        endif()
        message(FATAL_ERROR
          "slice2cpp_generate: '${header_file}' would be generated twice: once for ${owner}, and "
          "again for ${target}|${slice_file_path} (target|Slice file). Give the targets separate "
          "HEADER_OUTPUT_DIR directories, or add an INCLUDE_DIRS directory that tells the two "
          "Slice files apart.")
      endif()
      set_property(GLOBAL PROPERTY ${owner_property} "${target}|${slice_file_path}")

      # Prefix for the #include each generated source uses to reach its own header. It has to follow
      # the mirrored layout, since only the roots above are on the include path. A separate header
      # directory means the source can no longer find the header beside itself, so a mirrored file
      # needs the prefix even without INCLUDE_DIR.
      set(include_prefix "${arg_INCLUDE_DIR}")
      if(NOT slice_file_subdir STREQUAL ".")
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
          COMMAND ${CMAKE_COMMAND} -E copy ${file_output_dir}/${slice_file_name}.${header_ext} ${header_file}
          COMMAND ${CMAKE_COMMAND} -E rm -f ${file_output_dir}/${slice_file_name}.${header_ext})
      endif()

      add_custom_command(
        OUTPUT ${output_files}
        COMMAND ${CMAKE_COMMAND}
          -DSLICE2CPP=$<TARGET_FILE:Ice::slice2cpp>
          -DSLICE_FILE=${slice_file_path}
          -DSLICE_INCLUDE_DIRS=${include_dirs_arg}
          -DSLICE_OPTIONS=${options_arg}
          -DHEADER_DIR=${file_header_dir}
          -DEXPECTED_HEADER=${slice_file_name}.${header_ext}
          -DDEPFILE=${depfile}
          -P ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/slice2cpp_depend.cmake
        COMMAND $<TARGET_FILE:Ice::slice2cpp> ${include_options} ${include_dir_options} ${arg_OPTIONS}
          ${slice_file_path} --output-dir ${file_output_dir}
        ${move_header_commands}
        DEPENDS ${slice_file_path} $<TARGET_FILE:Ice::slice2cpp>
        DEPFILE ${depfile}
        VERBATIM
        COMMENT "Compiling Slice ${file} -> ${output_dir_relative}/${slice_file_name}.${source_ext} ${header_dir_relative}/${slice_file_name}.${header_ext}"
      )

      target_sources(${target} PRIVATE ${output_files})

    endif()
  endforeach()
endfunction()
