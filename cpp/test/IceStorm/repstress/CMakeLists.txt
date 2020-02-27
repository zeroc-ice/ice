if(NOT ${CMAKE_CURRENT_BINARY_DIR}/IceStorm)
	file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/IceStorm)
endif()

set(REPSTRESS_SLICES
	${CMAKE_CURRENT_SOURCE_DIR}/Controller.ice
	${CMAKE_CURRENT_SOURCE_DIR}/Single.ice
)

set(GENERATED_REPSTRESS_HEADERS 
	${CMAKE_CURRENT_BINARY_DIR}/IceStorm/Single.h
	${CMAKE_CURRENT_BINARY_DIR}/IceStorm/Controller.h
)

set(GENERATED_REPSTRESS_SOURCES 
	${CMAKE_CURRENT_BINARY_DIR}/IceStorm/Single.cpp
	${CMAKE_CURRENT_BINARY_DIR}/IceStorm/Controller.cpp
)

list(APPEND ICE_SLICE_DIRS "-I${CMAKE_CURRENT_SOURCE_DIR}")
list(APPEND ICE_SLICE_PARAMS ${COMPILE_SLICES_CPP_PARAMETERS} "IceStorm")

foreach(slice ${REPSTRESS_SLICES})
	get_filename_component(slice_name ${slice} NAME_WE)
	set(output_files 
		${CMAKE_CURRENT_BINARY_DIR}/IceStorm/${slice_name}.cpp 
		${CMAKE_CURRENT_BINARY_DIR}/IceStorm/${slice_name}.h
	)	
	#function(CompileSlice slice2bin_params slice_include_paths ice_file output_dir output_file)
	CompileSlice("${ICE_SLICE_PARAMS}"
		"${ICE_SLICE_DIRS}"
		${slice}
		${CMAKE_CURRENT_BINARY_DIR}/IceStorm
		"${output_files}"
	)
endforeach()

if(BUILD_ICE_CPP98)
	set(REPSTRESS_DEPS 
		ice
		icestorm
		testcommon
	)
elseif(BUILD_ICE_CPP11)
	set(REPSTRESS_DEPS 
		ice++11
		icestorm++11
		testcommon++11
	)
endif()

add_executable(icestormrepstresscontrol)

target_sources(icestormrepstresscontrol
	PRIVATE
		Control.cpp
		${GENERATED_REPSTRESS_HEADERS}
		${GENERATED_REPSTRESS_SOURCES}
)

target_include_directories(icestormrepstresscontrol
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/IceStorm
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET icestormrepstresscontrol PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET icestormrepstresscontrol PROPERTY CXX_STANDARD 11)
endif()

target_link_libraries(icestormrepstresscontrol PRIVATE ${REPSTRESS_DEPS})

target_compile_definitions(icestormrepstresscontrol PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Controller.ice
			Single.ice
	)

	source_group("Source Files/Generated"
		FILES
			${GENERATED_REPSTRESS_SOURCES}
	)

	source_group("Header Files/Generated"
		FILES
			${GENERATED_REPSTRESS_HEADERS}
	)

	target_compile_options(icestormrepstresscontrol
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	set_property(TARGET icestormrepstresscontrol PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
endif()

install(TARGETS icestormrepstresscontrol DESTINATION tests/bin)

add_executable(icestormrepstresspublisher)

target_sources(icestormrepstresspublisher
	PRIVATE
		Publisher.cpp
		${GENERATED_REPSTRESS_HEADERS}
		${GENERATED_REPSTRESS_SOURCES}
)

target_include_directories(icestormrepstresspublisher
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/IceStorm
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET icestormrepstresspublisher PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET icestormrepstresspublisher PROPERTY CXX_STANDARD 11)
endif()

target_link_libraries(icestormrepstresspublisher PRIVATE ${REPSTRESS_DEPS})

target_compile_definitions(icestormrepstresspublisher PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Controller.ice
			Single.ice
	)

	source_group("Source Files/Generated"
		FILES
			${GENERATED_REPSTRESS_SOURCES}
	)

	source_group("Header Files/Generated"
		FILES
			${GENERATED_REPSTRESS_HEADERS}
	)

	target_compile_options(icestormrepstresspublisher
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	set_property(TARGET icestormrepstresspublisher PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
endif()

install(TARGETS icestormrepstresspublisher DESTINATION tests/bin)

add_executable(icestormrepstresssubscriber)

target_sources(icestormrepstresssubscriber
	PRIVATE
		Subscriber.cpp
		${GENERATED_REPSTRESS_HEADERS}
		${GENERATED_REPSTRESS_SOURCES}
)

target_include_directories(icestormrepstresssubscriber
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/IceStorm
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET icestormrepstresssubscriber PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET icestormrepstresssubscriber PROPERTY CXX_STANDARD 11)
endif()

target_link_libraries(icestormrepstresssubscriber PRIVATE ${REPSTRESS_DEPS})

target_compile_definitions(icestormrepstresssubscriber PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Controller.ice
			Single.ice
	)

	source_group("Source Files/Generated"
		FILES
			${GENERATED_REPSTRESS_SOURCES}
	)

	source_group("Header Files/Generated"
		FILES
			${GENERATED_REPSTRESS_HEADERS}
	)

	target_compile_options(icestormrepstresssubscriber
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	set_property(TARGET icestormrepstresssubscriber PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
endif()

install(TARGETS icestormrepstresssubscriber DESTINATION tests/bin)
