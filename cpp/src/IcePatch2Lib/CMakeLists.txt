if(NOT BUILD_ICE_CPP11)
	if(NOT ${GENERATED_SLICES_DIR}/IcePatch2)
		file(MAKE_DIRECTORY ${GENERATED_SLICES_DIR}/IcePatch2)
	endif()

	set(GENERATED_ICEPATCH2_HEADERS
		${GENERATED_SLICES_DIR}/IcePatch2/FileInfo.h
		${GENERATED_SLICES_DIR}/IcePatch2/FileServer.h
	)

	set(GENERATED_ICEPATCH2_SOURCES
		${GENERATED_SLICES_DIR}/IcePatch2/FileInfo.cpp
		${GENERATED_SLICES_DIR}/IcePatch2/FileServer.cpp
	)

	list(APPEND ICEPATCH2_SLICE_DIRS "-I${SLICE_DIR}")
	list(APPEND ICEPATCH2_SLICE_PARAMS ${COMPILE_SLICES_CPP_PARAMETERS} "IcePatch2")

	foreach(slice IN LISTS ICEPATCH2_SLICES)
		#function(CompileSlice slice2bin_params slice_include_paths ice_file output_dir)
		CompileSlice("${ICEPATCH2_SLICE_PARAMS}"
			"${ICEPATCH2_SLICE_DIRS}"
			${slice}
			${GENERATED_SLICES_DIR}/IcePatch2
		)
	endforeach()

	install(DIRECTORY ${GENERATED_SLICES_DIR}/IcePatch2 
		DESTINATION ${GENERATED_H_INSTALL_DIR}
		FILES_MATCHING PATTERN "*.h"
	)	

	add_library(icepatch2)

	target_sources(icepatch2
		PRIVATE
			ClientUtil.cpp
			Util.cpp
			Util.h
			${GENERATED_ICEPATCH2_HEADERS}
			${GENERATED_ICEPATCH2_SOURCES}
	)

	target_include_directories(icepatch2
		PRIVATE 
			${GENERATED_SLICES_DIR}
			${PROJECT_SOURCE_DIR}/cpp/include
			${PROJECT_SOURCE_DIR}/cpp/src
			${BZIP2_INCLUDE_DIR} 
	)

	target_compile_definitions(icepatch2
		PRIVATE
			${ICE_COMPILE_DEFS}
			"ICEPATCH2_API_EXPORTS"
	)

	target_link_libraries(icepatch2
		PRIVATE
			ice
			BZip2::BZip2
	)

	if(MSVC)
		source_group(
			TREE ${CMAKE_CURRENT_SOURCE_DIR}
			PREFIX "Resource Files"
			FILES
				IcePatch2.rc
		)

		source_group(
			TREE ${GENERATED_SLICES_DIR}/IcePatch2
			PREFIX "Header Files/Generated"
			FILES
				${GENERATED_ICEPATCH2_HEADERS}
		)

		source_group(
			TREE ${GENERATED_SLICES_DIR}/IcePatch2
			PREFIX "Source Files/Generated"
			FILES
				${GENERATED_ICEPATCH2_SOURCES}
		)

		source_group(
			TREE ${SLICE_DIR}/IcePatch2
			PREFIX "Slice Files"
			FILES
				${ICEPATCH2_SLICES}
		)

		target_sources(icepatch2
			PRIVATE
				IcePatch2.rc
		)

		set_property(TARGET icepatch2 PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})

		target_compile_options(icepatch2
			PRIVATE
				${ICE_MSVC_COMPILE_OPTIONS}   
		)
	endif()

	set_target_properties(icepatch2
		PROPERTIES
			OUTPUT_NAME IcePatch2${zeroc-ice_VERSION_MAJOR}${zeroc-ice_VERSION_MINOR}
	)

	if(BUILD_SHARED_LIBS)
		install(TARGETS icepatch2 DESTINATION bin)
	else()
		install(TARGETS icepatch2 DESTINATION lib)
	endif()
endif()
