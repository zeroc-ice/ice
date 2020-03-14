set(icegrid_distribution_includes
	${PROJECT_SOURCE_DIR}/cpp/include
	${PROJECT_SOURCE_DIR}/cpp/test/include
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/IceGrid
	${CMAKE_CURRENT_BINARY_DIR}
	${GENERATED_SLICES_DIR}
)

set(distribution_client_sources
	${CMAKE_CURRENT_SOURCE_DIR}/AllTests.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Client.cpp
)

CreateExecutableIceTest(IceGrid
	"${icegrid_distribution_includes}"
	${ICEGRID_TEST_OUTPUT_DIR}/distribution
	"${distribution_client_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	icegriddistributionclient
	client
	"${ICEGRID_TEST_DEPS}"
)

set(distribution_server_sources
	${CMAKE_CURRENT_SOURCE_DIR}/Server.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.h
)

CreateExecutableIceTest(IceGrid
	"${icegrid_distribution_includes}"
	${ICEGRID_TEST_OUTPUT_DIR}/distribution
	"${distribution_server_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	icegriddistributionserver
	server
	"${ICEGRID_TEST_DEPS}"
)
