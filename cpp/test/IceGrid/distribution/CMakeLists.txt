if(NOT ${CMAKE_CURRENT_BINARY_DIR}/IceGrid)
	file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/IceGrid)
endif()

set(GENERATED_DISTRIBUTION_HEADERS ${CMAKE_CURRENT_BINARY_DIR}/IceGrid/Test.h)
set(GENERATED_DISTRIBUTION_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/IceGrid/Test.cpp)

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
	set(DISTRIBUTION_DEPS 
		ice
		glacier2
		icegrid
		testcommon
	)
elseif(BUILD_ICE_CPP11)
	set(DISTRIBUTION_DEPS 
		ice++11
		glacier2++11
		icegrid++11
		testcommon++11
	)
endif()

add_executable(icegriddistributionclient)

target_sources(icegriddistributionclient
	PRIVATE
		AllTests.cpp
		Client.cpp
		${GENERATED_DISTRIBUTION_HEADERS}
		${GENERATED_DISTRIBUTION_SOURCES}
)

target_include_directories(icegriddistributionclient
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/IceGrid
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET icegriddistributionclient PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET icegriddistributionclient PROPERTY CXX_STANDARD 11)
endif()

target_link_libraries(icegriddistributionclient PRIVATE ${DISTRIBUTION_DEPS})

target_compile_definitions(icegriddistributionclient PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Test.ice
	)

	source_group("Source Files/Generated"
		FILES
			${GENERATED_DISTRIBUTION_SOURCES}
	)

	source_group("Header Files/Generated"
		FILES
			${GENERATED_DISTRIBUTION_HEADERS}
	)

	target_compile_options(icegriddistributionclient
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	set_property(TARGET icegriddistributionclient PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
endif()

install(TARGETS icegriddistributionclient DESTINATION tests/bin)

add_executable(icegriddistributionserver)

target_sources(icegriddistributionserver
	PRIVATE
		Server.cpp
		TestI.cpp
		${GENERATED_DISTRIBUTION_HEADERS}
		${GENERATED_DISTRIBUTION_SOURCES}
)

target_include_directories(icegriddistributionserver
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/IceGrid
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET icegriddistributionserver PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET icegriddistributionserver PROPERTY CXX_STANDARD 11)
endif()

target_link_libraries(icegriddistributionserver PRIVATE ${DISTRIBUTION_DEPS})

target_compile_definitions(icegriddistributionserver PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Test.ice
	)

	source_group("Source Files/Generated"
		FILES
			${GENERATED_DISTRIBUTION_SOURCES}
	)

	source_group("Header Files/Generated"
		FILES
			${GENERATED_DISTRIBUTION_HEADERS}
	)

	target_compile_options(icegriddistributionserver
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	set_property(TARGET icegriddistributionserver PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
endif()

install(TARGETS icegriddistributionserver DESTINATION tests/bin)
