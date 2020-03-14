if(NOT BUILD_ICE_CPP11)
	add_executable(icebridge)

	target_sources(icebridge
		PRIVATE
			IceBridge.cpp
	)

	target_include_directories(icebridge
		PRIVATE
			${GENERATED_SLICES_DIR}
			${PROJECT_SOURCE_DIR}/cpp/include
			${PROJECT_SOURCE_DIR}/cpp/src
	)

	target_compile_definitions(icebridge
		PRIVATE
			${ICE_COMPILE_DEFS}
	)

	target_link_libraries(icebridge
		PRIVATE
			ice
	)

	if(MSVC)
		source_group(
			TREE ${CMAKE_CURRENT_SOURCE_DIR}
			PREFIX "Resource Files"
			FILES
				IceBridge.rc
		)
		target_sources(icebridge
			PRIVATE
				IceBridge.rc
		)

		target_compile_options(icebridge
			PRIVATE
				${ICE_MSVC_COMPILE_OPTIONS}
		)

		set_property(TARGET icebridge 
			PROPERTY 
				LINK_FLAGS "${ICE_MSVC_APP_LINK_OPTIONS} 
					${ICE_MSVC_DLL_LINK_OPTIONS}"
		)
	endif()

	install(TARGETS icebridge DESTINATION bin)
endif()
