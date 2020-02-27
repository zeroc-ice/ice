add_executable(iceutilsha1client)

target_sources(iceutilsha1client PRIVATE Client.cpp)

target_include_directories(iceutilsha1client
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/src
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${GENERATED_SLICES_DIR}
)

target_compile_definitions(iceutilsha1client PRIVATE ${ICE_COMPILE_DEFS})

target_link_libraries(iceutilsha1client 
	PRIVATE 
		testcommon
		ice
)

if(MSVC)
	target_compile_definitions(iceutilsha1client PRIVATE ${ICE_MSVC_COMPILE_DEFS})
	
	target_compile_options(iceutilsha1client PRIVATE ${ICE_MSVC_COMPILE_OPTIONS})

	set_property(TARGET iceutilsha1client PROPERTY LINK_FLAGS ${ICE_MSVC_LINK_OPTIONS})
endif()

install(TARGETS iceutilsha1client DESTINATION tests/bin)
