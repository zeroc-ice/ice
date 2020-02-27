add_executable(iceutilpriorityclient)

target_sources(iceutilpriorityclient 
	PRIVATE 
		Client.cpp
		PriorityInversion.cpp
		TestBase.cpp
		TestSuite.cpp
		ThreadPriority.cpp
		TimerPriority.cpp
)

target_include_directories(iceutilpriorityclient
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/src
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${GENERATED_SLICES_DIR}
		${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(iceutilpriorityclient PRIVATE ${ICE_COMPILE_DEFS})

target_link_libraries(iceutilpriorityclient 
	PRIVATE 
		testcommon
		ice
)

if(MSVC)
	target_compile_definitions(iceutilpriorityclient PRIVATE ${ICE_MSVC_COMPILE_DEFS})
	
	target_compile_options(iceutilpriorityclient PRIVATE ${ICE_MSVC_COMPILE_OPTIONS})

	set_property(TARGET iceutilpriorityclient PROPERTY LINK_FLAGS ${ICE_MSVC_LINK_OPTIONS})
endif()

install(TARGETS iceutilpriorityclient DESTINATION tests/bin)
