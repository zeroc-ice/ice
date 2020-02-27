if(BUILD_ICE_CPP98)
	set(PROPERTIES_DEPS 
		ice
		testcommon
	)
elseif(BUILD_ICE_CPP11)
	set(PROPERTIES_DEPS 
		ice++11
		testcommon++11
	)
endif()

add_executable(icegridadminserver)

target_sources(icegridadminserver
	PRIVATE
		Server.cpp
)

target_include_directories(icegridadminserver
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/src
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET icegridadminserver PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET icegridadminserver PROPERTY CXX_STANDARD 11)
endif()

target_link_libraries(icegridadminserver PRIVATE ${PROPERTIES_DEPS})

target_compile_definitions(icegridadminserver PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	target_compile_options(icegridadminserver
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	set_property(TARGET icegridadminserver PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
endif()

install(TARGETS icegridadminserver DESTINATION tests/bin)
