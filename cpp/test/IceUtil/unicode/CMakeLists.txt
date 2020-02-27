add_executable(iceutilunicodeclient)

target_sources(iceutilunicodeclient PRIVATE Client.cpp)

target_include_directories(iceutilunicodeclient
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/src
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${GENERATED_SLICES_DIR}
)

target_compile_definitions(iceutilunicodeclient PRIVATE ${ICE_COMPILE_DEFS})

target_link_libraries(iceutilunicodeclient 
	PRIVATE 
		testcommon
		ice
)

if(MSVC)
	target_compile_definitions(iceutilunicodeclient PRIVATE ${ICE_MSVC_COMPILE_DEFS})
	
	target_compile_options(iceutilunicodeclient PRIVATE ${ICE_MSVC_COMPILE_OPTIONS})

	set_property(TARGET iceutilunicodeclient PROPERTY LINK_FLAGS ${ICE_MSVC_LINK_OPTIONS})
endif()

install(TARGETS iceutilunicodeclient DESTINATION tests/bin)
