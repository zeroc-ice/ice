if(NOT BUILD_ICE_CPP11)
	if(NOT ${GENERATED_SLICES_DIR}/IceGrid)
		file(MAKE_DIRECTORY ${GENERATED_SLICES_DIR}/IceGrid)
	endif()

	set(ICEGRIDDB_SLICES
		${CMAKE_CURRENT_SOURCE_DIR}/DBTypes.ice
		${SLICE_DIR}/IceGrid/Descriptor.ice
		${SLICE_DIR}/IceGrid/Exception.ice
	)

	set(GENERATED_ICEGRIDDB_HEADERS
		${GENERATED_SLICES_DIR}/IceGrid/DBTypes.h
		${GENERATED_SLICES_DIR}/IceGrid/Descriptor.h
		${GENERATED_SLICES_DIR}/IceGrid/Exception.h
	)

	set(GENERATED_ICEGRIDDB_SOURCES
		${GENERATED_SLICES_DIR}/IceGrid/DBTypes.cpp
		${GENERATED_SLICES_DIR}/IceGrid/Descriptor.cpp
		${GENERATED_SLICES_DIR}/IceGrid/Exception.cpp
	)

	list(APPEND ICEGRIDDB_SLICE_DIRS "-I${SLICE_DIR}")
	list(APPEND ICEGRIDDB_SLICE_PARAMS ${COMPILE_SLICES_CPP_PARAMETERS} "IceGrid")

	foreach(slice IN LISTS ICEGRIDDB_SLICES)
		#function(CompileSlice slice2bin_params slice_include_paths ice_file output_dir)
		CompileSlice("${ICEGRIDDB_SLICE_PARAMS}"
			"${ICEGRIDDB_SLICE_DIRS}"
			${slice}
			${GENERATED_SLICES_DIR}/IceGrid
		)
	endforeach()

	install(DIRECTORY ${GENERATED_SLICES_DIR}/IceGrid 
		DESTINATION ${GENERATED_H_INSTALL_DIR}
		FILES_MATCHING PATTERN "*.h"
	)

	add_executable(icegriddb)

	add_dependencies(icegriddb icegrid)

	target_sources(icegriddb
		PRIVATE
			IceGridDB.cpp
			${GENERATED_ICEGRIDDB_HEADERS}
			${GENERATED_ICEGRIDDB_SOURCES}
	)

	target_include_directories(icegriddb
		PRIVATE 
			${GENERATED_SLICES_DIR}
			${PROJECT_SOURCE_DIR}/cpp/include
			${PROJECT_SOURCE_DIR}/cpp/src
			${LMDB_INCLUDE_DIR}
	)

	target_compile_definitions(icegriddb
		PRIVATE
			${ICE_COMPILE_DEFS}
			"ICE_BUILDING_ICE_GRID"
	)

	target_link_libraries(icegriddb
		PRIVATE
			ice
			icedb
			glacier2
			lmdb
	)

	if(MSVC)
		source_group(
			TREE ${CMAKE_CURRENT_SOURCE_DIR}
			PREFIX "Resource Files"
			FILES
				IceGridDB.rc
		)

		source_group(
			TREE ${GENERATED_SLICES_DIR}/IceGrid
			PREFIX "Header Files/Generated"
			FILES
				${GENERATED_ICEGRIDDB_HEADERS}
		)

		source_group(
			TREE ${GENERATED_SLICES_DIR}/IceGrid
			PREFIX "Source Files/Generated"
			FILES
				${GENERATED_ICEGRIDDB_SOURCES}
		)

		source_group("Slice Files"
			FILES
				${ICEGRIDDB_SLICES}
		)

		target_sources(icegriddb
			PRIVATE
				IceGridDB.rc
		)

		target_compile_options(icegriddb
			PRIVATE
				${ICE_MSVC_COMPILE_OPTIONS}   
		)

		set_property(TARGET icegriddb PROPERTY ${ICE_MSVC_APP_LINK_OPTIONS})
	endif()

	set_target_properties(icegriddb
		PROPERTIES
			OUTPUT_NAME IceGridDB
	)

	install(TARGETS icegriddb DESTINATION bin)
endif()
