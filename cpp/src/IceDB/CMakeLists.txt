if(NOT BUILD_ICE_CPP11)
	add_library(icedb)

	target_sources(icedb
		PRIVATE
			IceDB.cpp
			IceDB.h
	)

	target_include_directories(icedb
		PRIVATE
			${GENERATED_SLICES_DIR}
			${PROJECT_SOURCE_DIR}/cpp/include
			${PROJECT_SOURCE_DIR}/cpp/src
			${LMDB_INCLUDE_DIR}
	)

	target_compile_definitions(icedb
		PRIVATE 
			${ICE_COMPILE_DEFS}
			"ICE_DB_API_EXPORTS"
	)

	target_link_libraries(icedb
		PRIVATE
			ice
			lmdb
	)

	if(MSVC)
		source_group(
			TREE ${CMAKE_CURRENT_SOURCE_DIR}
			PREFIX "Resource Files"
			FILES
				IceDB.rc
		)

		target_sources(icedb
			PRIVATE
				IceDB.rc
		)

		set_property(TARGET icedb PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})

		target_compile_options(icedb
			PRIVATE
				${ICE_MSVC_COMPILE_OPTIONS}
		)
	endif()

	set_target_properties(icedb
		PROPERTIES
			OUTPUT_NAME IceDB${zeroc-ice_VERSION_MAJOR}${zeroc-ice_VERSION_MINOR}
	)

	if(BUILD_SHARED_LIBS)
		install(TARGETS icedb DESTINATION bin)
	else()
		install(TARGETS icedb DESTINATION lib)
	endif()
endif()
