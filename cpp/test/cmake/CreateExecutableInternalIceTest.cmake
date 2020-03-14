# CreateExecutableInternalIceTest - creates a test that uses a local ice file
#
# params: 
# test_name - the directory name of test/<name>
# include_dirs - include directories needed for the target
# output_dir - the directory the built files should go in
# test_sources - additional source files with path
# ice_files - ice files for the required test with path
# exe_target - name of the client target
# output_name - name for the built exe
# test_deps - the libraries the test depends on/links

function(CreateExecutableInternalIceTest test_name include_dirs output_dir test_sources ice_files exe_target output_name test_deps)
	get_filename_component(test_dir_name ${CMAKE_CURRENT_SOURCE_DIR} NAME)
	
	if(NOT ${output_dir}/${test_name})
		file(MAKE_DIRECTORY ${output_dir}/${test_name})
	endif()

	list(APPEND ICE_SLICE_DIRS "-I${CMAKE_CURRENT_SOURCE_DIR}")
	list(APPEND ICE_SLICE_PARAMS ${COMPILE_SLICES_CPP_PARAMETERS} ${test_name})

	foreach(slice IN LISTS ice_files)
		get_filename_component(slice_name ${slice} NAME_WE)

		list(APPEND GENERATED_EXEINTICE_HEADERS 
			${output_dir}/${test_name}/${slice_name}.h
		)

		list(APPEND GENERATED_EXEINTICE_SOURCES 
			${output_dir}/${test_name}/${slice_name}.cpp)

		#function(CompileSlice slice2bin_params slice_include_paths ice_files output_dir output_file)
		CompileSlice("${ICE_SLICE_PARAMS}"
			"${ICE_SLICE_DIRS}"
			${slice}
			${output_dir}/${test_name}
		)
	endforeach()

	add_executable(${exe_target})

	target_sources(${exe_target} 
		PRIVATE 
			${test_sources}
			${GENERATED_EXEINTICE_HEADERS}
			${GENERATED_EXEINTICE_SOURCES}
	)

	target_include_directories(${exe_target}
		PRIVATE
			${include_dirs}
	)

	if(BUILD_ICE_CPP11)
		set_property(TARGET ${exe_target} PROPERTY CXX_STANDARD 11)
	endif()

	target_link_libraries(${exe_target} PRIVATE ${test_deps})

	target_compile_definitions(${exe_target} PRIVATE ${ICE_COMPILE_DEFS})

	if(MSVC)
		source_group("Slice Files"
			FILES
				${ice_files}
		)

		source_group("Source Files/Generated"
			FILES
				${GENERATED_EXEINTICE_SOURCES}
		)

		source_group("Header Files/Generated"
			FILES
				${GENERATED_EXEINTICE_HEADERS}
		)

		target_compile_options(${exe_target}
			PRIVATE
				${ICE_MSVC_COMPILE_OPTIONS}
		)

		set_property(TARGET ${exe_target} PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
	endif()

	set_target_properties(${exe_target} 
		PROPERTIES 
			OUTPUT_NAME ${output_name}
			RUNTIME_OUTPUT_DIRECTORY ${output_dir}/bin
	)
endfunction()
