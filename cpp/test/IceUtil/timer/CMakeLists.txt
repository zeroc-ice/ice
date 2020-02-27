add_executable(iceutiltimerclient)

target_sources(iceutiltimerclient PRIVATE Client.cpp)

target_include_directories(iceutiltimerclient
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/src
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${GENERATED_SLICES_DIR}
)

target_compile_definitions(iceutiltimerclient PRIVATE ${ICE_COMPILE_DEFS})

target_link_libraries(iceutiltimerclient 
	PRIVATE 
		testcommon
		ice
)

if(MSVC)
	target_compile_definitions(iceutiltimerclient PRIVATE ${ICE_MSVC_COMPILE_DEFS})
	
	target_compile_options(iceutiltimerclient PRIVATE ${ICE_MSVC_COMPILE_OPTIONS})

	set_property(TARGET iceutiltimerclient PROPERTY LINK_FLAGS ${ICE_MSVC_LINK_OPTIONS})
endif()

install(TARGETS iceutiltimerclient DESTINATION tests/bin)
