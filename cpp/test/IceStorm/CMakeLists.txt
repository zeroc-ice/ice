if(BUILD_ICE_CPP11)
	set(ICESTORM_TEST_DEPS
		ice++11
		icestorm++11
		testcommon++11
	)
else()
	set(ICESTORM_TEST_DEPS
		ice
		icestorm
		testcommon
	)
endif()

set(ICESTORM_TEST_OUTPUT_DIR ${PROJECT_BINARY_DIR}/cpp/test/IceStorm)

add_subdirectory(federation)
add_subdirectory(federation2)
add_subdirectory(persistent)
add_subdirectory(rep1)
add_subdirectory(repgrid)
add_subdirectory(repstress)
add_subdirectory(single)
add_subdirectory(stress)
