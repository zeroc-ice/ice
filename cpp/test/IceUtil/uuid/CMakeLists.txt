add_executable(iceutiluuidclient)

target_sources(iceutiluuidclient PRIVATE Client.cpp)

target_include_directories(iceutiluuidclient
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/src
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${GENERATED_SLICES_DIR}
)

target_compile_definitions(iceutiluuidclient PRIVATE ${ICE_COMPILE_DEFS})

target_link_libraries(iceutiluuidclient 
	PRIVATE 
		testcommon
		ice
)

if(MSVC)
	target_compile_definitions(iceutiluuidclient PRIVATE ${ICE_MSVC_COMPILE_DEFS})
	
	target_compile_options(iceutiluuidclient PRIVATE ${ICE_MSVC_COMPILE_OPTIONS})

	set_property(TARGET iceutiluuidclient PROPERTY LINK_FLAGS ${ICE_MSVC_LINK_OPTIONS})
endif()

install(TARGETS iceutiluuidclient DESTINATION tests/bin)
