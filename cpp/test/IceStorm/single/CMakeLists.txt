if(NOT ${CMAKE_CURRENT_BINARY_DIR}/IceStorm)
	file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/IceStorm)
endif()

set(GENERATED_SINGLE_HEADERS ${CMAKE_CURRENT_BINARY_DIR}/IceStorm/Single.h)
set(GENERATED_SINGLE_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/IceStorm/Single.cpp)

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
	set(SINGLE_DEPS 
		ice
		icestorm
		testcommon
	)
elseif(BUILD_ICE_CPP11)
	set(SINGLE_DEPS 
		ice++11
		icestorm++11
		testcommon++11
	)
endif()

add_executable(icestormsinglepublisher)

target_sources(icestormsinglepublisher
	PRIVATE
		Publisher.cpp
		${GENERATED_SINGLE_HEADERS}
		${GENERATED_SINGLE_SOURCES}
)

target_include_directories(icestormsinglepublisher
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/IceStorm
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET icestormsinglepublisher PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET icestormsinglepublisher PROPERTY CXX_STANDARD 11)
endif()

target_link_libraries(icestormsinglepublisher PRIVATE ${SINGLE_DEPS})

target_compile_definitions(icestormsinglepublisher PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Single.ice
	)

	source_group("Source Files/Generated"
		FILES
			${GENERATED_SINGLE_SOURCES}
	)

	source_group("Header Files/Generated"
		FILES
			${GENERATED_SINGLE_HEADERS}
	)

	target_compile_options(icestormsinglepublisher
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	set_property(TARGET icestormsinglepublisher PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
endif()

install(TARGETS icestormsinglepublisher DESTINATION tests/bin)

add_executable(icestormsinglesubscriber)

target_sources(icestormsinglesubscriber
	PRIVATE
		Subscriber.cpp
		${GENERATED_SINGLE_HEADERS}
		${GENERATED_SINGLE_SOURCES}
)

target_include_directories(icestormsinglesubscriber
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/IceStorm
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET icestormsinglesubscriber PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET icestormsinglesubscriber PROPERTY CXX_STANDARD 11)
endif()

target_link_libraries(icestormsinglesubscriber PRIVATE ${SINGLE_DEPS})

target_compile_definitions(icestormsinglesubscriber PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Single.ice
	)

	source_group("Source Files/Generated"
		FILES
			${GENERATED_SINGLE_SOURCES}
	)

	source_group("Header Files/Generated"
		FILES
			${GENERATED_SINGLE_HEADERS}
	)

	target_compile_options(icestormsinglesubscriber
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	set_property(TARGET icestormsinglesubscriber PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
endif()

install(TARGETS icestormsinglesubscriber DESTINATION tests/bin)
