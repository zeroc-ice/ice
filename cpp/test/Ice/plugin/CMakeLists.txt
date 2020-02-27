if(BUILD_ICE_CPP98)
	set(PLUGIN_DEPS 
		ice
		testcommon
	)
elseif(BUILD_ICE_CPP11)
	set(PLUGIN_DEPS 
		ice++11
		testcommon++11
	)
endif()

add_executable(icepluginclient)

target_sources(icepluginclient
	PRIVATE
		Client.cpp
)

target_include_directories(icepluginclient
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/src
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/Ice
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET icepluginclient PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET icepluginclient PROPERTY CXX_STANDARD 11)
endif()

target_link_libraries(icepluginclient PRIVATE ${PLUGIN_DEPS})

target_compile_definitions(icepluginclient PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	target_compile_options(icepluginclient
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	set_property(TARGET icepluginclient PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
endif()

install(TARGETS icepluginclient DESTINATION tests/bin)

if(BUILD_ICE_CPP98)
	add_library(iceplugintestplugin)
	set_property(TARGET iceplugintestplugin
		PROPERTY
			OUTPUT_NAME testplugin${zeroc-ice_VERSION_MAJOR}${zeroc-ice_VERSION_MINOR}
	)
elseif(BUILD_ICE_CPP11)
	add_library(iceplugintestplugin++11)
	set_property(TARGET iceplugintestplugin++11
		PROPERTY
			OUTPUT_NAME testplugin${zeroc-ice_VERSION_MAJOR}${zeroc-ice_VERSION_MINOR}++11
	)
endif()

target_sources(iceplugintestplugin
	PRIVATE
		Plugin.cpp
)

target_include_directories(iceplugintestplugin
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/src
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/Ice
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET iceplugintestplugin PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET iceplugintestplugin PROPERTY CXX_STANDARD 11)
endif()

target_link_libraries(iceplugintestplugin PRIVATE ${PLUGIN_DEPS})

target_compile_definitions(iceplugintestplugin PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	set_property(TARGET iceplugintestplugin PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
endif()

if(BUILD_SHARED_LIBS)
	install(TARGETS iceplugintestplugin DESTINATION test/bin)
else()
	install(TARGETS iceplugintestplugin DESTINATION test/lib)
endif()
