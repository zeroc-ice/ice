if(NOT ${CMAKE_CURRENT_BINARY_DIR}/IceDiscovery)
	file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/IceDiscovery)
endif()

set(GENERATED_SIMPLE_HEADERS ${CMAKE_CURRENT_BINARY_DIR}/IceDiscovery/Test.h)
set(GENERATED_SIMPLE_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/IceDiscovery/Test.cpp)

list(APPEND ICE_SLICE_DIRS "-I${SLICE_DIR}")
list(APPEND ICE_SLICE_PARAMS ${COMPILE_SLICES_CPP_PARAMETERS} "IceDiscovery")

set(output_files 
	${CMAKE_CURRENT_BINARY_DIR}/IceDiscovery/Test.cpp 
	${CMAKE_CURRENT_BINARY_DIR}/IceDiscovery/Test.h
)	
#function(CompileSlice slice2bin_params slice_include_paths ice_file output_dir output_file)
CompileSlice("${ICE_SLICE_PARAMS}"
	"${ICE_SLICE_DIRS}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	${CMAKE_CURRENT_BINARY_DIR}/IceDiscovery
	"${output_files}"
)

if(BUILD_ICE_CPP98)
	set(SIMPLE_DEPS 
		ice
		icediscovery
		testcommon
	)
elseif(BUILD_ICE_CPP11)
	set(SIMPLE_DEPS 
		ice++11
		icediscovery++11
		testcommon++11
	)
endif()

add_executable(icediscoverysimpleclient)

target_sources(icediscoverysimpleclient
	PRIVATE
		AllTests.cpp
		Client.cpp
		${GENERATED_SIMPLE_HEADERS}
		${GENERATED_SIMPLE_SOURCES}
)

target_include_directories(icediscoverysimpleclient
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/IceDiscovery
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET icediscoverysimpleclient PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET icediscoverysimpleclient PROPERTY CXX_STANDARD 11)
endif()

target_link_libraries(icediscoverysimpleclient PRIVATE ${SIMPLE_DEPS})

target_compile_definitions(icediscoverysimpleclient PRIVATE ${ICE_COMPILE_DEFS})

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

	target_compile_options(icediscoverysimpleclient
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	set_property(TARGET icediscoverysimpleclient PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
endif()

install(TARGETS icediscoverysimpleclient DESTINATION tests/bin)

add_executable(icediscoverysimpleserver)

target_sources(icediscoverysimpleserver
	PRIVATE
		Server.cpp
		TestI.cpp
		${GENERATED_SIMPLE_HEADERS}
		${GENERATED_SIMPLE_SOURCES}
)

target_include_directories(icediscoverysimpleserver
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/IceDiscovery
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET icediscoverysimpleserver PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET icediscoverysimpleserver PROPERTY CXX_STANDARD 11)
endif()

target_link_libraries(icediscoverysimpleserver PRIVATE ${SIMPLE_DEPS})

target_compile_definitions(icediscoverysimpleserver PRIVATE ${ICE_COMPILE_DEFS})

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

	target_compile_options(icediscoverysimpleserver
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	set_property(TARGET icediscoverysimpleserver PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
endif()

install(TARGETS icediscoverysimpleserver DESTINATION tests/bin)
