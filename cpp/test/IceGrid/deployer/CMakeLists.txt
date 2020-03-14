set(icegrid_deployer_includes
	${PROJECT_SOURCE_DIR}/cpp/include
	${PROJECT_SOURCE_DIR}/cpp/test/include
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/IceGrid
	${CMAKE_CURRENT_BINARY_DIR}
	${GENERATED_SLICES_DIR}
)

set(deployer_client_sources
	${CMAKE_CURRENT_SOURCE_DIR}/AllTests.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Client.cpp
)

CreateExecutableIceTest(IceGrid
	"${icegrid_deployer_includes}"
	${ICEGRID_TEST_OUTPUT_DIR}/deployer
	"${deployer_client_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	icegriddeployerclient
	client
	"${ICEGRID_TEST_DEPS}"
)

set(deployer_server_sources
	${CMAKE_CURRENT_SOURCE_DIR}/Server.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.h
)

CreateExecutableIceTest(IceGrid
	"${icegrid_deployer_includes}"
	${ICEGRID_TEST_OUTPUT_DIR}/deployer
	"${deployer_server_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	icegriddeployerserver
	server
	"${ICEGRID_TEST_DEPS}"
)

set(deployer_service_sources
	${CMAKE_CURRENT_SOURCE_DIR}/Service.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.h
)

CreateLibraryIceTest(IceGrid
	"${icegrid_deployer_includes}"
	${ICEGRID_TEST_OUTPUT_DIR}/deployer
	"${deployer_service_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	icegriddeployertestservice
	testservice
	"${ICEGRID_TEST_SERVICE_DEPS}"
)
