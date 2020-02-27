if(NOT BUILD_ICE_CPP11)
	add_executable(iceserviceinstall)

	target_sources(iceserviceinstall 
		PRIVATE
			IceServiceInstall.rc
			Install.cpp
			ServiceInstaller.cpp
			ServiceInstaller.h
	)

	target_include_directories(iceserviceinstall
		PRIVATE
			${GENERATED_SLICES_DIR}
			${CMAKE_CURRENT_SOURCE_DIR}
			${PROJECT_SOURCE_DIR}/cpp/include
			${PROJECT_SOURCE_DIR}/cpp/src
	)

	target_compile_definitions(iceserviceinstall 
		PRIVATE 
			${ICE_COMPILE_DEFS}
	)

	target_link_libraries(iceserviceinstall 
		PRIVATE
			ice 
	)

	source_group(
		TREE ${CMAKE_CURRENT_SOURCE_DIR}
		PREFIX "Resource Files" 
		FILES IceServiceInstall.rc
	)

	target_compile_options(iceserviceinstall PRIVATE "/wd4250")

	set_property(TARGET iceserviceinstall PROPERTY 
		LINK_FLAGS "${ICE_MSVC_APP_LINK_OPTIONS} \
			${ICE_MSVC_DLL_LINK_OPTIONS}"
	)

	target_link_libraries(iceserviceinstall PRIVATE
		authz.lib
		advapi32.lib
	)

	install(TARGETS iceserviceinstall DESTINATION bin)
endif()
