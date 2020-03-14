add_library(icexml)

target_sources(icexml
	PRIVATE
		Parser.cpp
		Parser.h
)

target_include_directories(icexml
	PRIVATE
		${GENERATED_SLICES_DIR}
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/src
		${EXPAT_INCLUDE_DIR}
)

target_compile_definitions(icexml
	PRIVATE
		${ICE_COMPILE_DEFS}
		"ICE_XML_API_EXPORTS"
)

target_link_libraries(icexml
	PRIVATE
		ice
		EXPAT::EXPAT
)

if(MSVC)
	source_group(
		TREE ${CMAKE_CURRENT_SOURCE_DIR}
		PREFIX "Resource Files"
		FILES
			IceXML.rc
	)

	target_sources(icexml
		PRIVATE
			IceXML.rc
	)

	set_property(TARGET icexml PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})

	target_compile_options(icexml
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)
endif()

set_target_properties(icexml 
	PROPERTIES
		OUTPUT_NAME IceXML${zeroc-ice_VERSION_MAJOR}${zeroc-ice_VERSION_MINOR}
)

if(BUILD_SHARED_LIBS)
	install(TARGETS icexml DESTINATION bin)
else()
	install(TARGETS icexml DESTINATION lib)
endif()
