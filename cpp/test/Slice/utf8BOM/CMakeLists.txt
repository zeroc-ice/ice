if(NOT ${CMAKE_CURRENT_BINARY_DIR}/Slice)
	file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Slice)
endif()

set(GENERATED_UTF8BOM_HEADERS ${CMAKE_CURRENT_BINARY_DIR}/Slice/Test.h)
set(GENERATED_UTF8BOM_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/Slice/Test.cpp)

list(APPEND ICE_SLICE_DIRS "-I${SLICE_DIR}")
list(APPEND ICE_SLICE_PARAMS ${COMPILE_SLICES_CPP_PARAMETERS} "Slice")

set(output_files 
	${CMAKE_CURRENT_BINARY_DIR}/Slice/Test.cpp 
	${CMAKE_CURRENT_BINARY_DIR}/Slice/Test.h
)	
#function(CompileSlice slice2bin_params slice_include_paths ice_file output_dir output_file)
CompileSlice("${ICE_SLICE_PARAMS}"
	"${ICE_SLICE_DIRS}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	${CMAKE_CURRENT_BINARY_DIR}/Slice
	"${output_files}"
)

add_library(sliceutf8bomclient STATIC)

target_sources(sliceutf8bomclient
	PRIVATE
		${GENERATED_UTF8BOM_HEADERS}
		${GENERATED_UTF8BOM_SOURCES}
)

target_include_directories(sliceutf8bomclient
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/Slice
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET sliceutf8bomclient PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET sliceutf8bomclient PROPERTY CXX_STANDARD 11)
endif()

target_link_libraries(sliceutf8bomclient 
	PRIVATE 
		testcommon 
		ice
)

target_compile_definitions(sliceutf8bomclient PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Test.ice
	)

	source_group("Source Files/Generated"
		FILES
			${GENERATED_UTF8BOM_SOURCES}
	)

	source_group("Header Files/Generated"
		FILES
			${GENERATED_UTF8BOM_HEADERS}
	)

	target_compile_options(sliceutf8bomclient
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	set_property(TARGET sliceutf8bomclient PROPERTY LINK_FLAGS ${ICE_MSVC_LINK_OPTIONS})
endif()

install(TARGETS sliceutf8bomclient DESTINATION tests/lib)
