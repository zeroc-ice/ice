add_executable(iceutilctrlchandlerclient)

target_sources(iceutilctrlchandlerclient PRIVATE Client.cpp)

target_include_directories(iceutilctrlchandlerclient
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/src
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${GENERATED_SLICES_DIR}
)

target_compile_definitions(iceutilctrlchandlerclient PRIVATE ${ICE_COMPILE_DEFS})

target_link_libraries(iceutilctrlchandlerclient 
	PRIVATE 
		testcommon
		ice
)

if(MSVC)
	target_compile_definitions(iceutilctrlchandlerclient PRIVATE ${ICE_MSVC_COMPILE_DEFS})
	
	target_compile_options(iceutilctrlchandlerclient PRIVATE ${ICE_MSVC_COMPILE_OPTIONS})

	set_property(TARGET iceutilctrlchandlerclient PROPERTY LINK_FLAGS ${ICE_MSVC_LINK_OPTIONS})
endif()

install(TARGETS iceutilctrlchandlerclient DESTINATION tests/bin)
