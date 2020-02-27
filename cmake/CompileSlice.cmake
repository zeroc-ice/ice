# CompileSlice - script slice compile with cmake similar to IceBuilder
# 
# Params:
# slice2bin_params - parameters passed for specific slice2_bin use cases
# slice_include_path - include dir(s) needed for -I parameter
# ice_file - file to be compiled (<filename>.ice)
# output_dir - generated source output path

function(CompileSlice slice2bin_params slice_include_path ice_file output_dir)
	message(STATUS "Generating sources for ${ice_file}...")

	execute_process(COMMAND ${SLICE2_BIN}
			${slice_include_path} 
			${slice2bin_params} 
			${ice_file}
		WORKING_DIRECTORY ${output_dir}
	)

	set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${ice_file})
endfunction()
