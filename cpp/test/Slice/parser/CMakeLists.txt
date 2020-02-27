if(NOT ${CMAKE_CURRENT_BINARY_DIR}/Slice)
	file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Slice)
endif()

set(PARSER_SLICES
	${CMAKE_CURRENT_SOURCE_DIR}/CircularA.ice
	${CMAKE_CURRENT_SOURCE_DIR}/CircularB.ice
)

set(GENERATED_PARSER_HEADERS
	${CMAKE_CURRENT_BINARY_DIR}/Slice/CircularA.h
	${CMAKE_CURRENT_BINARY_DIR}/Slice/CircularB.h
)

set(GENERATED_PARSER_SOURCES
	${CMAKE_CURRENT_BINARY_DIR}/Slice/CircularA.cpp
	${CMAKE_CURRENT_BINARY_DIR}/Slice/CircularB.cpp
)

list(APPEND ICE_SLICE_DIRS "-I${CMAKE_CURRENT_SOURCE_DIR}")
list(APPEND ICE_SLICE_PARAMS ${COMPILE_SLICES_CPP_PARAMETERS} "Slice")

foreach(slice ${PARSER_SLICES})
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

add_library(sliceparserclient)

target_sources(sliceparserclient 
	PRIVATE 
		${GENERATED_PARSER_HEADERS}
		${GENERATED_PARSER_SOURCES}
)

target_include_directories(sliceparserclient
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/src
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_BINARY_DIR}/Slice
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

target_compile_definitions(sliceparserclient PRIVATE ${ICE_COMPILE_DEFS})

target_link_libraries(sliceparserclient 
	PRIVATE 
		testcommon
		ice
)

if(MSVC)
	source_group("Header Files/Generated"
		FILES
			${GENERATED_PARSER_HEADERS}
	)

	source_group("Source Files/Generated"
		FILES
			${GENERATED_PARSER_SOURCES}
	)

	source_group("Slice Files"
		FILES
			${PARSER_SLICES}
	)

	target_compile_definitions(sliceparserclient PRIVATE ${ICE_MSVC_COMPILE_DEFS})
	
	target_compile_options(sliceparserclient PRIVATE ${ICE_MSVC_COMPILE_OPTIONS})

	set_property(TARGET sliceparserclient PROPERTY LINK_FLAGS ${ICE_MSVC_LINK_OPTIONS})
endif()

install(TARGETS sliceparserclient DESTINATION tests/lib)
