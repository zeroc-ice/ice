if(NOT ${GENERATED_SLICES_DIR}/IceGrid)
	file(MAKE_DIRECTORY ${GENERATED_SLICES_DIR}/IceGrid)
endif()

set(GENERATED_ICEGRID_HEADERS
	${GENERATED_SLICES_DIR}/IceGrid/Admin.h
	${GENERATED_SLICES_DIR}/IceGrid/Descriptor.h
	${GENERATED_SLICES_DIR}/IceGrid/Exception.h
	${GENERATED_SLICES_DIR}/IceGrid/FileParser.h
	${GENERATED_SLICES_DIR}/IceGrid/PluginFacade.h
	${GENERATED_SLICES_DIR}/IceGrid/Registry.h
	${GENERATED_SLICES_DIR}/IceGrid/Session.h
	${GENERATED_SLICES_DIR}/IceGrid/UserAccountMapper.h
)

set(GENERATED_ICEGRID_SOURCES
	${GENERATED_SLICES_DIR}/IceGrid/Admin.cpp
	${GENERATED_SLICES_DIR}/IceGrid/Descriptor.cpp
	${GENERATED_SLICES_DIR}/IceGrid/Exception.cpp
	${GENERATED_SLICES_DIR}/IceGrid/FileParser.cpp
	${GENERATED_SLICES_DIR}/IceGrid/PluginFacade.cpp
	${GENERATED_SLICES_DIR}/IceGrid/Registry.cpp
	${GENERATED_SLICES_DIR}/IceGrid/Session.cpp
	${GENERATED_SLICES_DIR}/IceGrid/UserAccountMapper.cpp
)

set(ICEGRID_SOURCES
	PluginFacadeI.cpp
)

list(APPEND ICEGRID_SLICE_DIRS "-I${SLICE_DIR}")
list(APPEND ICEGRID_SLICE_PARAMS ${COMPILE_SLICES_CPP_PARAMETERS} "IceGrid")

foreach(slice IN LISTS ICEGRID_SLICES)
	#function(CompileSlice slice2bin_params slice_include_paths ice_file output_dir)
	CompileSlice("${ICEGRID_SLICE_PARAMS}"
		"${ICEGRID_SLICE_DIRS}"
		${slice}
		${GENERATED_SLICES_DIR}/IceGrid
	)
endforeach()

install(DIRECTORY ${GENERATED_SLICES_DIR}/IceGrid 
	DESTINATION ${GENERATED_H_INSTALL_DIR}
	FILES_MATCHING PATTERN "*.h"
)

if(BUILD_ICE_CPP11)
	set(ICEGRID_TARGET icegrid++11)

	add_library(${ICEGRID_TARGET})

	set_target_properties(${ICEGRID_TARGET} 
		PROPERTIES
			OUTPUT_NAME IceGrid${zeroc-ice_VERSION_MAJOR}${zeroc-ice_VERSION_MINOR}++11
			CXX_STANDARD 11
	)

	target_link_libraries(${ICEGRID_TARGET}
		PRIVATE
			glacier2++11
			ice++11
	)
else()
	set(ICEGRID_TARGET icegrid)

	add_library(${ICEGRID_TARGET})

	set_target_properties(${ICEGRID_TARGET} 
		PROPERTIES
			OUTPUT_NAME IceGrid${zeroc-ice_VERSION_MAJOR}${zeroc-ice_VERSION_MINOR}
	)

	target_link_libraries(${ICEGRID_TARGET}
		PRIVATE
			glacier2
			ice
	)
endif()

target_sources(${ICEGRID_TARGET}
	PRIVATE
		${ICEGRID_SOURCES} 
		${GENERATED_ICEGRID_HEADERS}
		${GENERATED_ICEGRID_SOURCES}
)

target_include_directories(${ICEGRID_TARGET}
	PRIVATE 
		${GENERATED_SLICES_DIR}
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/src  
)

target_compile_definitions(${ICEGRID_TARGET}
	PRIVATE
		${ICE_COMPILE_DEFS}
		"ICEGRID_API_EXPORTS"
)

if(MSVC)
	source_group(
		TREE ${CMAKE_CURRENT_SOURCE_DIR}
		PREFIX "Resource Files"
		FILES
			IceGrid.rc
	)

	target_sources(${ICEGRID_TARGET}
		PRIVATE
			IceGrid.rc
	)

	source_group(
		TREE ${GENERATED_SLICES_DIR}/IceGrid
		PREFIX "Header Files/Generated"
		FILES
			${GENERATED_ICEGRID_HEADERS}
	)

	source_group(
		TREE ${GENERATED_SLICES_DIR}/IceGrid
		PREFIX "Source Files/Generated"
		FILES
			${GENERATED_ICEGRID_SOURCES}
	)

	source_group(
		TREE ${SLICE_DIR}/IceGrid
		PREFIX "Slice Files"
		FILES
			${ICEGRID_SLICES}
	)

	set_property(TARGET ${ICEGRID_TARGET} PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})

	target_compile_options(${ICEGRID_TARGET}
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}   
	)
endif()

if(BUILD_SHARED_LIBS)
	install(TARGETS ${ICEGRID_TARGET} DESTINATION bin)
else()
	install(TARGETS ${ICEGRID_TARGET} DESTINATION lib)
endif()
