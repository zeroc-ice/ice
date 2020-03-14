set(icegrid_simple_includes
	${PROJECT_SOURCE_DIR}/cpp/include
	${PROJECT_SOURCE_DIR}/cpp/test/include
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/IceGrid
	${CMAKE_CURRENT_BINARY_DIR}
	${GENERATED_SLICES_DIR}
)

set(simple_client_sources
	${CMAKE_CURRENT_SOURCE_DIR}/AllTests.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Client.cpp
)

if(BUILD_ICE_CPP11)
	set(simple_client_deps
		icelocatordiscovery++11
		${ICEGRID_TEST_DEPS}
	)
else()
	set(simple_client_deps
		icelocatordiscovery
		${ICEGRID_TEST_DEPS}
	)
endif()

CreateExecutableIceTest(IceGrid
	"${icegrid_simple_includes}"
	${ICEGRID_TEST_OUTPUT_DIR}/simple
	"${simple_client_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	icegridsimpleclient
	client
	"${simple_client_deps}"
)

set(simple_server_sources
	${CMAKE_CURRENT_SOURCE_DIR}/Server.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.h
)

if(BUILD_ICE_CPP11)
	set(simple_server_deps
		icelocatordiscovery++11
		${ICEGRID_TEST_DEPS}
	)
else()
	set(simple_server_deps
		icelocatordiscovery
		${ICEGRID_TEST_DEPS}
	)
endif()

CreateExecutableIceTest(IceGrid
	"${icegrid_simple_includes}"
	${ICEGRID_TEST_OUTPUT_DIR}/simple
	"${simple_server_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	icegridsimpleserver
	server
	"${simple_server_deps}"
)
