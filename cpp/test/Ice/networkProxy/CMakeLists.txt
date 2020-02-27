if(NOT ${CMAKE_CURRENT_BINARY_DIR}/Ice)
	file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Ice)
endif()

set(GENERATED_NETWORKPROXY_HEADERS ${CMAKE_CURRENT_BINARY_DIR}/Ice/Test.h)
set(GENERATED_NETWORKPROXY_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/Ice/Test.cpp)

list(APPEND ICE_SLICE_DIRS "-I${SLICE_DIR}")
list(APPEND ICE_SLICE_PARAMS ${COMPILE_SLICES_CPP_PARAMETERS} "Ice")

set(output_files 
	${CMAKE_CURRENT_BINARY_DIR}/Ice/Test.cpp 
	${CMAKE_CURRENT_BINARY_DIR}/Ice/Test.h
)	
#function(CompileSlice slice2bin_params slice_include_paths ice_file output_dir output_file)
CompileSlice("${ICE_SLICE_PARAMS}"
	"${ICE_SLICE_DIRS}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	${CMAKE_CURRENT_BINARY_DIR}/Ice
	"${output_files}"
)

if(BUILD_ICE_CPP98)
	set(NETWORKPROXY_DEPS 
		ice
		testcommon
	)
elseif(BUILD_ICE_CPP11)
	set(NETWORKPROXY_DEPS 
		ice++11
		testcommon++11
	)
elseif(BUILD_ICE_UWP)
	set(NETWORKPROXY_DEPS 
		iceuwp++11
		testcommonuwp++11
	)
endif()

add_executable(icenetworkproxyclient)

target_sources(icenetworkproxyclient
	PRIVATE
		AllTests.cpp
		Client.cpp
		${GENERATED_NETWORKPROXY_HEADERS}
		${GENERATED_NETWORKPROXY_SOURCES}
)

target_include_directories(icenetworkproxyclient
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/Ice
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET icenetworkproxyclient PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET icenetworkproxyclient PROPERTY CXX_STANDARD 11)
endif()

if(BUILD_ICE_UWP)
	add_dependencies(icenetworkproxyclient ${NETWORKPROXY_DEPS})
	set_property(TARGET icenetworkproxyclient PROPERTY CXX_STANDARD 11)
else()
	target_link_libraries(icenetworkproxyclient PRIVATE ${NETWORKPROXY_DEPS})
endif()

target_compile_definitions(icenetworkproxyclient PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Test.ice
	)

	source_group("Source Files/Generated"
		FILES
			${GENERATED_NETWORKPROXY_SOURCES}
	)

	source_group("Header Files/Generated"
		FILES
			${GENERATED_NETWORKPROXY_HEADERS}
	)

	target_compile_options(icenetworkproxyclient
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	if(NOT BUILD_ICE_UWP)
		set_property(TARGET icenetworkproxyclient PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
	else()
		set_property(TARGET icenetworkproxyclient PROPERTY LINK_FLAGS ${ICE_UWP_LINK_OPTIONS})
	endif()
endif()

install(TARGETS icenetworkproxyclient DESTINATION tests/bin)

add_executable(icenetworkproxyserver)

target_sources(icenetworkproxyserver
	PRIVATE
		Server.cpp
		${GENERATED_NETWORKPROXY_HEADERS}
		${GENERATED_NETWORKPROXY_SOURCES}
)

target_include_directories(icenetworkproxyserver
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/Ice
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET icenetworkproxyserver PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET icenetworkproxyserver PROPERTY CXX_STANDARD 11)
endif()

if(BUILD_ICE_UWP)
	add_dependencies(icenetworkproxyserver ${NETWORKPROXY_DEPS})
	set_property(TARGET icenetworkproxyserver PROPERTY CXX_STANDARD 11)
else()
	target_link_libraries(icenetworkproxyserver PRIVATE ${NETWORKPROXY_DEPS})
endif()

target_compile_definitions(icenetworkproxyserver PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Test.ice
	)

	source_group("Source Files/Generated"
		FILES
			${GENERATED_NETWORKPROXY_SOURCES}
	)

	source_group("Header Files/Generated"
		FILES
			${GENERATED_NETWORKPROXY_HEADERS}
	)

	target_compile_options(icenetworkproxyserver
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	if(NOT BUILD_ICE_UWP)
		set_property(TARGET icenetworkproxyserver PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
	else()
		set_property(TARGET icenetworkproxyserver PROPERTY LINK_FLAGS ${ICE_UWP_LINK_OPTIONS})
	endif()
endif()

install(TARGETS icenetworkproxyserver DESTINATION tests/bin)
