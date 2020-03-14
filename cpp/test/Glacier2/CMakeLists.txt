if(BUILD_ICE_CPP11)
	set(GLACIER2_TEST_DEPS 
		glacier2++11 
		ice++11
		icediscovery++11
		icelocatordiscovery++11
		icessl++11
		testcommon++11
	)
else()
	set(GLACIER2_TEST_DEPS 
		glacier2 
		ice
		icediscovery
		icelocatordiscovery
		icessl
		testcommon
	)
endif()

set(GLACIER2_TEST_OUTPUT_DIR ${PROJECT_BINARY_DIR}/cpp/test/Glacier2)

add_subdirectory(application)
add_subdirectory(sessionHelper)

if(NOT BUILD_ICE_CPP11)
	add_subdirectory(attack)
	add_subdirectory(override)
	add_subdirectory(router)
	add_subdirectory(sessionControl)
	add_subdirectory(ssl)

	if(BUILD_SHARED_LIBS)
		add_subdirectory(dynamicFiltering)
	else()
		add_subdirectory(staticFiltering)
	endif()
endif()
