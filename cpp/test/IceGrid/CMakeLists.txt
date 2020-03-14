if(BUILD_ICE_CPP11)
	set(ICEGRID_TEST_DEPS
		ice++11
		glacier2++11
		icegrid++11
		testcommon++11
	)
	set(ICEGRID_TEST_SERVICE_DEPS
		ice++11
		iceboxlib++11
		testcommon++11
	)
else()
	set(ICEGRID_TEST_DEPS
		ice
		glacier2
		icegrid
		testcommon
	)

	set(ICEGRID_TEST_SERVICE_DEPS
		ice
		iceboxlib
		testcommon
	)
endif()

set(ICEGRID_TEST_OUTPUT_DIR ${PROJECT_BINARY_DIR}/cpp/test/IceGrid)

add_subdirectory(admin)
add_subdirectory(simple)

if(NOT BUILD_ICE_CPP11)
	add_subdirectory(activation)
	add_subdirectory(allocation)
	add_subdirectory(deployer)
	add_subdirectory(distribution)
	add_subdirectory(noRestartUpdate)
	add_subdirectory(replicaGroup)
	add_subdirectory(replication)
	add_subdirectory(session)
	add_subdirectory(update)
endif()
