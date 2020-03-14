if(BUILD_ICE_CPP11)
	set(ICEBRIDGE_TEST_DEPS
		ice++11
		testcommon++11
	)
else()
	set(ICEBRIDGE_TEST_DEPS
		ice
		testcommon
	)
endif()

set(ICEBRIDGE_TEST_OUTPUT_DIR ${PROJECT_BINARY_DIR}/cpp/test/IceBridge)


add_subdirectory(simple)
