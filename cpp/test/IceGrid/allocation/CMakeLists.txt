set(icegrid_allocation_includes
	${PROJECT_SOURCE_DIR}/cpp/include
	${PROJECT_SOURCE_DIR}/cpp/test/include
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/IceGrid
	${CMAKE_CURRENT_BINARY_DIR}
	${GENERATED_SLICES_DIR}
)

set(allocation_client_sources
	${CMAKE_CURRENT_SOURCE_DIR}/AllTests.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Client.cpp
)

CreateExecutableIceTest(IceGrid
	"${icegrid_allocation_includes}"
	${ICEGRID_TEST_OUTPUT_DIR}/allocation
	"${allocation_client_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	icegridallocationclient
	client
	"${ICEGRID_TEST_DEPS}"
)

set(allocation_server_sources
	${CMAKE_CURRENT_SOURCE_DIR}/Server.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.h
)

CreateExecutableIceTest(IceGrid
	"${icegrid_allocation_includes}"
	${ICEGRID_TEST_OUTPUT_DIR}/allocation
	"${allocation_server_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	icegridallocationserver
	server
	"${ICEGRID_TEST_DEPS}"
)

CreateExecutableIceTest(IceGrid
	"${icegrid_allocation_includes}"
	${ICEGRID_TEST_OUTPUT_DIR}/allocation
	${CMAKE_CURRENT_SOURCE_DIR}/PermissionsVerifier.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	icegridallocationverifier
	verifier
	"${ICEGRID_TEST_DEPS}"
)
