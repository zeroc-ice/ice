if(BUILD_ICE_CPP11)
	set(ICEBOX_TEST_SERVICE_DEPS
		ice++11
		iceboxlib++11
		testcommon++11
	)

	set(ICEBOX_TEST_DEPS
		ice++11
		testcommon++11
	)
else()
	set(ICEBOX_TEST_SERVICE_DEPS
		ice
		iceboxlib
		testcommon
	)

	set(ICEBOX_TEST_DEPS
		ice
		testcommon
	)
endif()

set(ICEBOX_TEST_OUTPUT_DIR ${PROJECT_BINARY_DIR}/cpp/test/IceBox)

add_subdirectory(admin)
add_subdirectory(configuration)
