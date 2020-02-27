if(NOT ${CMAKE_CURRENT_BINARY_DIR}/IceStorm)
	file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/IceStorm)
endif()

set(GENERATED_REP1_HEADERS ${CMAKE_CURRENT_BINARY_DIR}/IceStorm/Single.h)
set(GENERATED_REP1_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/IceStorm/Single.cpp)

list(APPEND ICE_SLICE_DIRS "-I${SLICE_DIR}")
list(APPEND ICE_SLICE_PARAMS ${COMPILE_SLICES_CPP_PARAMETERS} "IceStorm")

set(output_files 
	${CMAKE_CURRENT_BINARY_DIR}/IceStorm/Single.cpp 
	${CMAKE_CURRENT_BINARY_DIR}/IceStorm/Single.h
)	
#function(CompileSlice slice2bin_params slice_include_paths ice_file output_dir output_file)
CompileSlice("${ICE_SLICE_PARAMS}"
	"${ICE_SLICE_DIRS}"
	${CMAKE_CURRENT_SOURCE_DIR}/Single.ice
	${CMAKE_CURRENT_BINARY_DIR}/IceStorm
	"${output_files}"
)

if(BUILD_ICE_CPP98)
	set(REP1_DEPS 
		ice
		icestorm
		testcommon
	)
elseif(BUILD_ICE_CPP11)
	set(REP1_DEPS 
		ice++11
		icestorm++11
		testcommon++11
	)
endif()

add_executable(icestormrep1publisher)

target_sources(icestormrep1publisher
	PRIVATE
		Publisher.cpp
		${GENERATED_REP1_HEADERS}
		${GENERATED_REP1_SOURCES}
)

target_include_directories(icestormrep1publisher
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/IceStorm
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET icestormrep1publisher PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET icestormrep1publisher PROPERTY CXX_STANDARD 11)
endif()

target_link_libraries(icestormrep1publisher PRIVATE ${REP1_DEPS})

target_compile_definitions(icestormrep1publisher PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Single.ice
	)

	source_group("Source Files/Generated"
		FILES
			${GENERATED_REP1_SOURCES}
	)

	source_group("Header Files/Generated"
		FILES
			${GENERATED_REP1_HEADERS}
	)

	target_compile_options(icestormrep1publisher
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	set_property(TARGET icestormrep1publisher PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
endif()

install(TARGETS icestormrep1publisher DESTINATION tests/bin)

add_executable(icestormrep1sub)

target_sources(icestormrep1sub
	PRIVATE
		Sub.cpp
		${GENERATED_REP1_HEADERS}
		${GENERATED_REP1_SOURCES}
)

target_include_directories(icestormrep1sub
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/IceStorm
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET icestormrep1sub PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET icestormrep1sub PROPERTY CXX_STANDARD 11)
endif()

target_link_libraries(icestormrep1sub PRIVATE ${REP1_DEPS})

target_compile_definitions(icestormrep1sub PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Single.ice
	)

	source_group("Source Files/Generated"
		FILES
			${GENERATED_REP1_SOURCES}
	)

	source_group("Header Files/Generated"
		FILES
			${GENERATED_REP1_HEADERS}
	)

	target_compile_options(icestormrep1sub
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	set_property(TARGET icestormrep1sub PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
endif()

install(TARGETS icestormrep1sub DESTINATION tests/bin)

add_executable(icestormrep1subscriber)

target_sources(icestormrep1subscriber
	PRIVATE
		Subscriber.cpp
		${GENERATED_REP1_HEADERS}
		${GENERATED_REP1_SOURCES}
)

target_include_directories(icestormrep1subscriber
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/IceStorm
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET icestormrep1subscriber PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET icestormrep1subscriber PROPERTY CXX_STANDARD 11)
endif()

target_link_libraries(icestormrep1subscriber PRIVATE ${REP1_DEPS})

target_compile_definitions(icestormrep1subscriber PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Single.ice
	)

	source_group("Source Files/Generated"
		FILES
			${GENERATED_REP1_SOURCES}
	)

	source_group("Header Files/Generated"
		FILES
			${GENERATED_REP1_HEADERS}
	)

	target_compile_options(icestormrep1subscriber
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	set_property(TARGET icestormrep1subscriber PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
endif()

install(TARGETS icestormrep1subscriber DESTINATION tests/bin)
