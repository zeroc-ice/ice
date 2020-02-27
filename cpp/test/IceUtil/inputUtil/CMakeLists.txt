add_executable(iceutilinpututilclient)

target_sources(iceutilinpututilclient PRIVATE Client.cpp)

target_include_directories(iceutilinpututilclient
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/src
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${GENERATED_SLICES_DIR}
)

target_compile_definitions(iceutilinpututilclient PRIVATE ${ICE_COMPILE_DEFS})

target_link_libraries(iceutilinpututilclient 
	PRIVATE 
		testcommon
		ice
)

if(MSVC)
	target_compile_definitions(iceutilinpututilclient PRIVATE ${ICE_MSVC_COMPILE_DEFS})
	
	target_compile_options(iceutilinpututilclient PRIVATE ${ICE_MSVC_COMPILE_OPTIONS})

	set_property(TARGET iceutilinpututilclient PROPERTY LINK_FLAGS ${ICE_MSVC_LINK_OPTIONS})
endif()

install(TARGETS iceutilinpututilclient DESTINATION tests/bin)
