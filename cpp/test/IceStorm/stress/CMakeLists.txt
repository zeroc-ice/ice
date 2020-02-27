if(NOT ${CMAKE_CURRENT_BINARY_DIR}/IceStorm)
	file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/IceStorm)
endif()

set(GENERATED_STRESS_HEADERS ${CMAKE_CURRENT_BINARY_DIR}/IceStorm/Event.h)
set(GENERATED_STRESS_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/IceStorm/Event.cpp)

list(APPEND ICE_SLICE_DIRS "-I${SLICE_DIR}")
list(APPEND ICE_SLICE_PARAMS ${COMPILE_SLICES_CPP_PARAMETERS} "IceStorm")

set(output_files 
	${CMAKE_CURRENT_BINARY_DIR}/IceStorm/Event.cpp 
	${CMAKE_CURRENT_BINARY_DIR}/IceStorm/Event.h
)	
#function(CompileSlice slice2bin_params slice_include_paths ice_file output_dir output_file)
CompileSlice("${ICE_SLICE_PARAMS}"
	"${ICE_SLICE_DIRS}"
	${CMAKE_CURRENT_SOURCE_DIR}/Event.ice
	${CMAKE_CURRENT_BINARY_DIR}/IceStorm
	"${output_files}"
)

if(BUILD_ICE_CPP98)
	set(STRESS_DEPS 
		ice
		icestorm
		testcommon
	)
elseif(BUILD_ICE_CPP11)
	set(STRESS_DEPS 
		ice++11
		icestorm++11
		testcommon++11
	)
endif()

add_executable(icestormstresspublisher)

target_sources(icestormstresspublisher
	PRIVATE
		Publisher.cpp
		${GENERATED_STRESS_HEADERS}
		${GENERATED_STRESS_SOURCES}
)

target_include_directories(icestormstresspublisher
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/IceStorm
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET icestormstresspublisher PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET icestormstresspublisher PROPERTY CXX_STANDARD 11)
endif()

target_link_libraries(icestormstresspublisher PRIVATE ${STRESS_DEPS})

target_compile_definitions(icestormstresspublisher PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Event.ice
	)

	source_group("Source Files/Generated"
		FILES
			${GENERATED_STRESS_SOURCES}
	)

	source_group("Header Files/Generated"
		FILES
			${GENERATED_STRESS_HEADERS}
	)

	target_compile_options(icestormstresspublisher
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	set_property(TARGET icestormstresspublisher PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
endif()

install(TARGETS icestormstresspublisher DESTINATION tests/bin)

add_executable(icestormstresssubscriber)

target_sources(icestormstresssubscriber
	PRIVATE
		Subscriber.cpp
		${GENERATED_STRESS_HEADERS}
		${GENERATED_STRESS_SOURCES}
)

target_include_directories(icestormstresssubscriber
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/IceStorm
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET icestormstresssubscriber PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET icestormstresssubscriber PROPERTY CXX_STANDARD 11)
endif()

target_link_libraries(icestormstresssubscriber PRIVATE ${STRESS_DEPS})

target_compile_definitions(icestormstresssubscriber PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Event.ice
	)

	source_group("Source Files/Generated"
		FILES
			${GENERATED_STRESS_SOURCES}
	)

	source_group("Header Files/Generated"
		FILES
			${GENERATED_STRESS_HEADERS}
	)

	target_compile_options(icestormstresssubscriber
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	set_property(TARGET icestormstresssubscriber PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
endif()

install(TARGETS icestormstresssubscriber DESTINATION tests/bin)
