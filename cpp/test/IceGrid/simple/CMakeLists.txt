if(NOT ${CMAKE_CURRENT_BINARY_DIR}/IceGrid)
	file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/IceGrid)
endif()

set(GENERATED_SIMPLE_HEADERS ${CMAKE_CURRENT_BINARY_DIR}/IceGrid/Test.h)
set(GENERATED_SIMPLE_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/IceGrid/Test.cpp)

list(APPEND ICE_SLICE_DIRS "-I${SLICE_DIR}")
list(APPEND ICE_SLICE_PARAMS ${COMPILE_SLICES_CPP_PARAMETERS} "IceGrid")

set(output_files 
	${CMAKE_CURRENT_BINARY_DIR}/IceGrid/Test.cpp 
	${CMAKE_CURRENT_BINARY_DIR}/IceGrid/Test.h
)	
#function(CompileSlice slice2bin_params slice_include_paths ice_file output_dir output_file)
CompileSlice("${ICE_SLICE_PARAMS}"
	"${ICE_SLICE_DIRS}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	${CMAKE_CURRENT_BINARY_DIR}/IceGrid
	"${output_files}"
)

if(BUILD_ICE_CPP98)
	set(SIMPLE_DEPS 
		ice
		glacier2
		icegrid
		icelocatordiscovery
		testcommon
	)
elseif(BUILD_ICE_CPP11)
	set(SIMPLE_DEPS 
		ice++11
		glacier2++11
		icegrid++11
		icelocatordiscovery++11
		testcommon++11
	)
endif()

add_executable(icegridsimpleclient)

target_sources(icegridsimpleclient
	PRIVATE
		AllTests.cpp
		Client.cpp
		${GENERATED_SIMPLE_HEADERS}
		${GENERATED_SIMPLE_SOURCES}
)

target_include_directories(icegridsimpleclient
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/IceGrid
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET icegridsimpleclient PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET icegridsimpleclient PROPERTY CXX_STANDARD 11)
endif()

target_link_libraries(icegridsimpleclient PRIVATE ${SIMPLE_DEPS})

target_compile_definitions(icegridsimpleclient PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Test.ice
	)

	source_group("Source Files/Generated"
		FILES
			${GENERATED_SIMPLE_SOURCES}
	)

	source_group("Header Files/Generated"
		FILES
			${GENERATED_SIMPLE_HEADERS}
	)

	target_compile_options(icegridsimpleclient
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	set_property(TARGET icegridsimpleclient PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
endif()

install(TARGETS icegridsimpleclient DESTINATION tests/bin)

add_executable(icegridsimpleserver)

target_sources(icegridsimpleserver
	PRIVATE
		Server.cpp
		TestI.cpp
		${GENERATED_SIMPLE_HEADERS}
		${GENERATED_SIMPLE_SOURCES}
)

target_include_directories(icegridsimpleserver
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/IceGrid
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET icegridsimpleserver PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET icegridsimpleserver PROPERTY CXX_STANDARD 11)
endif()

target_link_libraries(icegridsimpleserver PRIVATE ${SIMPLE_DEPS})

target_compile_definitions(icegridsimpleserver PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Test.ice
	)

	source_group("Source Files/Generated"
		FILES
			${GENERATED_SIMPLE_SOURCES}
	)

	source_group("Header Files/Generated"
		FILES
			${GENERATED_SIMPLE_HEADERS}
	)

	target_compile_options(icegridsimpleserver
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	set_property(TARGET icegridsimpleserver PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
endif()

install(TARGETS icegridsimpleserver DESTINATION tests/bin)
