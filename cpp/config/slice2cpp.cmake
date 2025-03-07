# Copyright (c) ZeroC, Inc.

# Function to generate C++ source files from Slice (.ice) files for a target using slice2cpp
# The target must have the Slice files in its sources
# The generated files are added to the target sources
# Usage:
# add_executable(a_target source1.cpp source2.ice source3.ice)
# slice2cpp_generate(a_target)
function(slice2cpp_generate target)

  # Get the list of source files for the target
  get_target_property(sources ${target} SOURCES)

  # Get the output directory for the generated files
  set(output_dir ${CMAKE_CURRENT_BINARY_DIR}/generated/${target})
  file(RELATIVE_PATH output_dir_relative ${CMAKE_CURRENT_LIST_DIR} ${output_dir})

  # Create a directory to store the generated files
  make_directory(${output_dir})

  # Add the generated headers files to the target include directories
  target_include_directories(${target} PRIVATE ${output_dir})

  # Process each Slice (.ice) file in the source list
  # 1. Run the slice2cpp command to generate the header and source files
  # 2. Add the generated files to the target sources
  foreach(file IN LISTS sources)
    if(file MATCHES "\\.ice$")

      get_filename_component(slice_file_name ${file} NAME_WE)
      get_filename_component(slice_file_path ${file} ABSOLUTE)

      set(output_files ${output_dir}/${slice_file_name}.h ${output_dir}/${slice_file_name}.cpp)

      add_custom_command(
        OUTPUT ${output_files}
        COMMAND $<TARGET_FILE:Ice::slice2cpp> -I${Ice_SLICE_DIR} ${slice_file_path} --output-dir ${output_dir}
        DEPENDS ${slice_file_path}
        COMMENT "Compiling Slice ${file} -> ${output_dir_relative}/${slice_file_name}.cpp ${output_dir_relative}/${slice_file_name}.h"
      )

      target_sources(${target} PRIVATE ${output_files})

    endif()
  endforeach()
endfunction()
