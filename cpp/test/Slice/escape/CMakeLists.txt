if(NOT ${CMAKE_CURRENT_BINARY_DIR}/Slice)
	file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Slice)
endif()

set(ESCAPE_SLICES
	${CMAKE_CURRENT_SOURCE_DIR}/Clash.ice
	${CMAKE_CURRENT_SOURCE_DIR}/Key.ice
)

set(GENERATED_ESCAPE_HEADERS
	${CMAKE_CURRENT_BINARY_DIR}/Slice/Clash.h
	${CMAKE_CURRENT_BINARY_DIR}/Slice/Key.h
)

set(GENERATED_ESCAPE_SOURCES
	${CMAKE_CURRENT_BINARY_DIR}/Slice/Clash.cpp
	${CMAKE_CURRENT_BINARY_DIR}/Slice/Key.cpp
)

list(APPEND ICE_SLICE_DIRS "-I${SLICE_DIR}")
list(APPEND ICE_SLICE_PARAMS ${COMPILE_SLICES_CPP_PARAMETERS} "Slice")

foreach(slice ${ESCAPE_SLICES})
	get_filename_component(slice_name ${slice} NAME_WE)
	set(output_files 
		${CMAKE_CURRENT_BINARY_DIR}/Slice/${slice_name}.cpp 
		${CMAKE_CURRENT_BINARY_DIR}/Slice/${slice_name}.h
	)	
	#function(CompileSlice slice2bin_params slice_include_paths ice_file output_dir output_file)
	CompileSlice("${ICE_SLICE_PARAMS}"
		"${ICE_SLICE_DIRS}"
		${slice}
		${CMAKE_CURRENT_BINARY_DIR}/Slice
		"${output_files}"
	)
endforeach()

add_executable(sliceescapeclient)

target_sources(sliceescapeclient 
	PRIVATE 
		Client.cpp
		${GENERATED_ESCAPE_HEADERS}
		${GENERATED_ESCAPE_SOURCES}
)

target_include_directories(sliceescapeclient
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/src
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_BINARY_DIR}/Slice
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

target_compile_definitions(sliceescapeclient PRIVATE ${ICE_COMPILE_DEFS})

target_link_libraries(sliceescapeclient 
	PRIVATE 
		testcommon
		ice
)

if(MSVC)
	source_group("Header Files/Generated"
		FILES
			${GENERATED_ESCAPE_HEADERS}
	)

	source_group("Source Files/Generated"
		FILES
			${GENERATED_ESCAPE_SOURCES}
	)

	source_group("Slice Files"
		FILES
			${ESCAPE_SLICES}
	)

	target_compile_definitions(sliceescapeclient PRIVATE ${ICE_MSVC_COMPILE_DEFS})
	
	target_compile_options(sliceescapeclient PRIVATE ${ICE_MSVC_COMPILE_OPTIONS})

	set_property(TARGET sliceescapeclient PROPERTY LINK_FLAGS ${ICE_MSVC_LINK_OPTIONS})
endif()

install(TARGETS sliceescapeclient DESTINATION tests/bin)
