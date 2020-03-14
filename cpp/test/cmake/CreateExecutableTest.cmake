# CreateExecutableTest - creates a test for a client
#
# params: 
# test_name - the directory name of test/<name>
# include_dirs - include directories needed by the target
# output_dir - the directory the built files should go in
# test_sources - the test sources with path
# exe_target - name of the client target
# output_name - name of the built target
# test_deps - the targets or libraries the test depends on/links

function(CreateExecutableTest test_name include_dirs output_dir test_sources exe_target output_name test_deps)
	add_executable(${exe_target})

	target_sources(${exe_target} 
		PRIVATE 
			${test_sources}
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
