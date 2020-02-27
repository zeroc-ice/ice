if(NOT BUILD_ICE_CPP11)
	add_library(glacier2cryptpermissionsverifier)

	target_sources(glacier2cryptpermissionsverifier
		PRIVATE
			CryptPermissionsVerifierI.cpp
	)

	target_include_directories(glacier2cryptpermissionsverifier
		PRIVATE 
			${GENERATED_SLICES_DIR}
			${PROJECT_SOURCE_DIR}/cpp/include
			${PROJECT_SOURCE_DIR}/cpp/src  
	)

	target_compile_definitions(glacier2cryptpermissionsverifier
		PRIVATE
			${ICE_COMPILE_DEFS}
			"CRYPT_PERMISSIONS_VERIFIER_API_EXPORTS"
	)

	target_link_libraries(glacier2cryptpermissionsverifier
		PRIVATE
			glacier2
			ice
	)

	if(MSVC)
		source_group(
			TREE ${CMAKE_CURRENT_SOURCE_DIR}
			PREFIX "Resource Files"
			FILES
				CryptPermissionsVerifier.rc
		)

		target_compile_options(glacier2cryptpermissionsverifier
			PRIVATE
				${ICE_MSVC_COMPILE_OPTIONS}   
		)

		target_link_libraries(glacier2cryptpermissionsverifier
			PRIVATE
				crypt32.lib
				bcrypt.lib
		)

		set_property(TARGET glacier2cryptpermissionsverifier
			PROPERTY
				LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS}
		)

		target_sources(glacier2cryptpermissionsverifier
			PRIVATE
				CryptPermissionsVerifier.rc
		)
	endif()

	set_target_properties(glacier2cryptpermissionsverifier 
		PROPERTIES
			OUTPUT_NAME Glacier2CryptPermissionsVerifier${zeroc-ice_VERSION_MAJOR}${zeroc-ice_VERSION_MINOR}
	)

	if(BUILD_SHARED_LIBS)
		install(TARGETS glacier2cryptpermissionsverifier 
			DESTINATION bin
		)
	else()
		install(TARGETS glacier2cryptpermissionsverifier 
			DESTINATION lib
		)
	endif()
endif()
