add_executable(iceutilstacktraceclient)

target_sources(iceutilstacktraceclient PRIVATE Client.cpp)

target_include_directories(iceutilstacktraceclient
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/src
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${GENERATED_SLICES_DIR}
)

target_compile_definitions(iceutilstacktraceclient PRIVATE ${ICE_COMPILE_DEFS})

target_link_libraries(iceutilstacktraceclient 
	PRIVATE 
		testcommon
		ice
)

if(MSVC)
	target_compile_definitions(iceutilstacktraceclient PRIVATE ${ICE_MSVC_COMPILE_DEFS})
	
	target_compile_options(iceutilstacktraceclient PRIVATE ${ICE_MSVC_COMPILE_OPTIONS})
	
	target_link_libraries(iceutilstacktraceclient PRIVATE advapi32.lib)

	set_property(TARGET iceutilstacktraceclient PROPERTY LINK_FLAGS ${ICE_MSVC_LINK_OPTIONS})
endif()

install(TARGETS iceutilstacktraceclient DESTINATION tests/bin)
