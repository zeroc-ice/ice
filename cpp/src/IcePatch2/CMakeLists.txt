if(NOT BUILD_ICE_CPP11)
	add_executable(icepatch2calc)

	target_sources(icepatch2calc
		PRIVATE
			Calc.cpp
	)

	target_include_directories(icepatch2calc
		PRIVATE
			${GENERATED_SLICES_DIR}
			${PROJECT_SOURCE_DIR}/cpp/include
			${PROJECT_SOURCE_DIR}/cpp/src
	)

	target_compile_definitions(icepatch2calc
		PRIVATE
			${ICE_COMPILE_DEFS}
	)

	target_link_libraries(icepatch2calc
		PRIVATE
			icepatch2
			ice
	)

	if(UNIX)
		target_link_libraries(icepatch2calc PRIVATE icesslopenssl)
	endif()

	if(MSVC)
		source_group(
			TREE ${CMAKE_CURRENT_SOURCE_DIR}
			PREFIX "Resource Files"
			FILES
				IcePatch2Calc.rc
		)

		target_sources(icepatch2calc
			PRIVATE
				IcePatch2Calc.rc
		)

		target_compile_options(icepatch2calc
			PRIVATE
				${ICE_MSVC_COMPILE_OPTIONS} 
		)

		set_property(TARGET icepatch2calc PROPERTY LINK_FLAGS ${ICE_MSVC_APP_LINK_OPTIONS})
	endif()

	install(TARGETS icepatch2calc DESTINATION bin)

	add_executable(icepatch2client)

	target_sources(icepatch2client
		PRIVATE
			Client.cpp
	)

	target_include_directories(icepatch2client
		PRIVATE
			${GENERATED_SLICES_DIR}
			${PROJECT_SOURCE_DIR}/cpp/include
			${PROJECT_SOURCE_DIR}/cpp/src
	)

	target_compile_definitions(icepatch2client
		PRIVATE
			${ICE_COMPILE_DEFS}
	)

	target_link_libraries(icepatch2client
		PRIVATE
			icepatch2
			ice
	)

	if(UNIX)
		target_link_libraries(icepatch2client PRIVATE icesslopenssl)
	endif()

	if(MSVC)
		source_group(
			TREE ${CMAKE_CURRENT_SOURCE_DIR}
			PREFIX "Resource Files"
			FILES
				IcePatch2Client.rc
		)

		target_sources(icepatch2client
			PRIVATE
				IcePatch2Client.rc
		)

		target_compile_options(icepatch2client
			PRIVATE
				${ICE_MSVC_COMPILE_OPTIONS} 
		)

		target_compile_definitions(icepatch2client
			PRIVATE
				${ICE_WIN32_COMPILE_DEFS} 
		)

		set_property(TARGET icepatch2client PROPERTY LINK_FLAGS ${ICE_MSVC_APP_LINK_OPTIONS})
	endif()

	install(TARGETS icepatch2client DESTINATION bin)

	add_executable(icepatch2server)

	target_sources(icepatch2server
		PRIVATE
			FileServerI.cpp
			Server.cpp
	)

	target_include_directories(icepatch2server
		PRIVATE
			${GENERATED_SLICES_DIR}
			${PROJECT_SOURCE_DIR}/cpp/include
			${PROJECT_SOURCE_DIR}/cpp/src
	)

	target_compile_definitions(icepatch2server
		PRIVATE
			${ICE_COMPILE_DEFS}
	)

	target_link_libraries(icepatch2server
		PRIVATE
			icepatch2
			ice
	)

	if(UNIX)
		target_link_libraries(icepatch2server PRIVATE icesslopenssl)
	endif()

	if(MSVC)
		source_group(
			TREE ${CMAKE_CURRENT_SOURCE_DIR}
			PREFIX "Resource Files"
			FILES
				IcePatch2Server.rc
		)

		target_sources(icepatch2server
			PRIVATE
				IcePatch2Server.rc
		)

		target_compile_options(icepatch2server
			PRIVATE
				${ICE_MSVC_COMPILE_OPTIONS} 
		)

		target_compile_definitions(icepatch2server
			PRIVATE
				${ICE_WIN32_COMPILE_DEFS} 
		)

		set_property(TARGET icepatch2server PROPERTY LINK_FLAGS ${ICE_MSVC_APP_LINK_OPTIONS})
	endif()

	install(TARGETS icepatch2server DESTINATION bin)
endif()
