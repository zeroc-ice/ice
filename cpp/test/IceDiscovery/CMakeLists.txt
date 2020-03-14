if(BUILD_ICE_CPP11)
	set(ICEDISCOVERY_TEST_DEPS
		ice++11
		icediscovery++11
		testcommon++11
	)
else()
	set(ICEDISCOVERY_TEST_DEPS
		ice
		icediscovery
		testcommon
	)
endif()

set(ICEDISCOVERY_TEST_OUTPUT_DIR ${PROJECT_BINARY_DIR}/cpp/test/IceDiscovery)

add_subdirectory(simple)
