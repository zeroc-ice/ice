# CreateLibraryTest - creates a test that uses a Test ice file
#
# params: 
# test_name - the directory name of test/<name>
# include_dirs - include directories needed for the target
# output_dir - the directory the built files should go in
# test_sources - the test source files with path
# lib_target - name of the library target
# lib_name - name of the output file
# test_deps - the targets or libraries the test depends on

function(CreateLibraryTest test_name include_dirs output_dir test_sources lib_target lib_name test_deps)
	add_library(${lib_target})

	target_sources(${lib_target} 
		PRIVATE 
			${test_sources}
	)

	target_include_directories(${lib_target}
		PRIVATE
			${include_dirs}
	)

	if(BUILD_ICE_CPP11)
		set_property(TARGET ${lib_target} PROPERTY CXX_STANDARD 11)
	endif()

	target_link_libraries(${lib_target} PRIVATE ${test_deps})

	target_compile_definitions(${lib_target} PRIVATE ${ICE_COMPILE_DEFS})

	if(MSVC)
		target_compile_options(${lib_target}
			PRIVATE
				${ICE_MSVC_COMPILE_OPTIONS}
		)

		set_property(TARGET ${lib_target} PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
	endif()

	if(BUILD_ICE_CPP11)
		set_target_properties(${lib_target}
			PROPERTIES
				OUTPUT_NAME ${lib_name}${zeroc-ice_VERSION_MAJOR}${zeroc-ice_VERSION_MINOR}++11
				CXX_STANDARD 11
		)
	else()
		set_property(TARGET ${lib_target}
			PROPERTY
				OUTPUT_NAME ${lib_name}${zeroc-ice_VERSION_MAJOR}${zeroc-ice_VERSION_MINOR}
		)
	endif()

	if(BUILD_SHARED_LIBS)
		set_property(TARGET ${lib_target} PROPERTY RUNTIME_OUTPUT_DIRECTORY ${output_dir}/bin)
	else()
		set_property(TARGET ${lib_target} PROPERTY LIBRARY_OUTPUT_DIRECTORY ${output_dir}/lib)
	endif()
endfunction()
