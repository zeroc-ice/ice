if(NOT ${GENERATED_SLICES_DIR}/IceStorm)
	file(MAKE_DIRECTORY ${GENERATED_SLICES_DIR}/IceStorm)
endif()

set(GENERATED_ICESTORM_HEADERS
	${GENERATED_SLICES_DIR}/IceStorm/IceStorm.h
	${GENERATED_SLICES_DIR}/IceStorm/Metrics.h
)

set(GENERATED_ICESTORM_SOURCES
	${GENERATED_SLICES_DIR}/IceStorm/IceStorm.cpp
	${GENERATED_SLICES_DIR}/IceStorm/Metrics.cpp
)

list(APPEND ICESTORM_SLICE_DIRS "-I${SLICE_DIR}")
list(APPEND ICESTORM_SLICE_PARAMS ${COMPILE_SLICES_CPP_PARAMETERS} "IceStorm")

foreach(slice IN LISTS ICESTORM_SLICES)
	#function(CompileSlice slice2bin_params slice_include_paths ice_file output_dir)
	CompileSlice("${ICESTORM_SLICE_PARAMS}"
		"${ICESTORM_SLICE_DIRS}"
		${slice}
		${GENERATED_SLICES_DIR}/IceStorm
	)
endforeach()

install(DIRECTORY ${GENERATED_SLICES_DIR}/IceStorm 
	DESTINATION ${GENERATED_H_INSTALL_DIR}
	FILES_MATCHING PATTERN "*.h"
)

if(BUILD_ICE_CPP11)
	set(ICESTORMLIB_TARGET icestorm++11)

	add_library(${ICESTORMLIB_TARGET})

	set_target_properties(${ICESTORMLIB_TARGET} 
		PROPERTIES
			OUTPUT_NAME IceStorm${zeroc-ice_VERSION_MAJOR}${zeroc-ice_VERSION_MINOR}++11
			CXX_STANDARD 11
	)

	target_link_libraries(${ICESTORMLIB_TARGET} PRIVATE ice++11)
else()
	set(ICESTORMLIB_TARGET icestorm)

	add_library(${ICESTORMLIB_TARGET})

	set_target_properties(${ICESTORMLIB_TARGET} 
		PROPERTIES
			OUTPUT_NAME IceStorm${zeroc-ice_VERSION_MAJOR}${zeroc-ice_VERSION_MINOR}
	)

	target_link_libraries(${ICESTORMLIB_TARGET} PRIVATE ice)
endif()

target_sources(${ICESTORMLIB_TARGET}
	PRIVATE
		${GENERATED_ICESTORM_HEADERS}
		${GENERATED_ICESTORM_SOURCES}
)

target_include_directories(${ICESTORMLIB_TARGET}
	PRIVATE 
		${GENERATED_SLICES_DIR}
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/src
)

target_compile_definitions(${ICESTORMLIB_TARGET}
	PRIVATE
		${ICE_COMPILE_DEFS}
		"ICESTORM_API_EXPORTS"
)

if(MSVC)
	source_group(
		TREE ${CMAKE_CURRENT_SOURCE_DIR}
		PREFIX "Resource Files"
		FILES
			IceStorm.rc
	)

	target_sources(${ICESTORMLIB_TARGET}
		PRIVATE
			IceStorm.rc
	)

	source_group(
		TREE ${GENERATED_SLICES_DIR}/IceStorm
		PREFIX "Header Files/Generated"
		FILES
			${GENERATED_ICESTORM_HEADERS}
	)

	source_group(
		TREE ${GENERATED_SLICES_DIR}/IceStorm
		PREFIX "Source Files/Generated"
		FILES
			${GENERATED_ICESTORM_SOURCES}
	)

	source_group(
		TREE ${SLICE_DIR}/IceStorm
		PREFIX "Slice Files"
		FILES
			${ICESTORM_SLICES}
	)

	set_property(TARGET ${ICESTORMLIB_TARGET} PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})

	target_compile_options(${ICESTORMLIB_TARGET} 
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}   
	)
endif()

if(BUILD_SHARED_LIBS)
	install(TARGETS ${ICESTORMLIB_TARGET} DESTINATION bin)
else()
	install(TARGETS ${ICESTORMLIB_TARGET} DESTINATION lib)
endif()
