set(icediscovery_simple_includes
	${PROJECT_SOURCE_DIR}/cpp/include
	${PROJECT_SOURCE_DIR}/cpp/test/include
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/IceDiscovery
	${CMAKE_CURRENT_BINARY_DIR}
	${GENERATED_SLICES_DIR}
)

set(simple_client_sources
	${CMAKE_CURRENT_SOURCE_DIR}/AllTests.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Client.cpp
)

CreateExecutableIceTest(IceDiscovery
	"${icediscovery_simple_includes}"
	${ICEDISCOVERY_TEST_OUTPUT_DIR}/simple
	"${simple_client_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	icediscoverysimpleclient
	client
	"${ICEDISCOVERY_TEST_DEPS}"
)

set(simple_server_sources
	${CMAKE_CURRENT_SOURCE_DIR}/Server.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.h
)

CreateExecutableIceTest(IceDiscovery
	"${icediscovery_simple_includes}"
	${ICEDISCOVERY_TEST_OUTPUT_DIR}/simple
	"${simple_server_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	icediscoverysimpleserver
	server
	"${ICEDISCOVERY_TEST_DEPS}"
)
