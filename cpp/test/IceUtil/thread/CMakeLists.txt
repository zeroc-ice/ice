add_executable(iceutilthreadclient)

target_sources(iceutilthreadclient 
	PRIVATE 
		AliveTest.cpp
		Client.cpp
		CountDownLatchTest.cpp
		CreateTest.cpp
		MonitorMutexTest.cpp
		MonitorRecMutexTest.cpp
		MutexTest.cpp
		RecMutexTest.cpp
		SleepTest.cpp
		StartTest.cpp
		TestBase.cpp
		TestSuite.cpp
)

target_include_directories(iceutilthreadclient
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/src
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${GENERATED_SLICES_DIR}
		${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(iceutilthreadclient PRIVATE ${ICE_COMPILE_DEFS})

target_link_libraries(iceutilthreadclient 
	PRIVATE 
		testcommon
		ice
)

if(MSVC)
	target_compile_definitions(iceutilthreadclient PRIVATE ${ICE_MSVC_COMPILE_DEFS})
	
	target_compile_options(iceutilthreadclient PRIVATE ${ICE_MSVC_COMPILE_OPTIONS})

	set_property(TARGET iceutilthreadclient PROPERTY LINK_FLAGS ${ICE_MSVC_LINK_OPTIONS})
endif()

install(TARGETS iceutilthreadclient DESTINATION tests/bin)
