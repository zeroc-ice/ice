set(ice_checksum_includes
	${PROJECT_SOURCE_DIR}/cpp/include
	${PROJECT_SOURCE_DIR}/cpp/test/include
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/Ice
	${CMAKE_CURRENT_BINARY_DIR}
	${GENERATED_SLICES_DIR}
)

set(checksum_client_slices
	${CMAKE_CURRENT_SOURCE_DIR}/ClientTypes.ice
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
)

set(checksum_client_sources
	${CMAKE_CURRENT_SOURCE_DIR}/AllTests.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Client.cpp
)

CreateExecutableIceTest(Ice
	"${ice_checksum_includes}"
	${ICE_TEST_OUTPUT_DIR}/checksum 
	"${checksum_client_sources}"
	"${checksum_client_slices}"
	icechecksumclient
	client 
	"${ICE_TEST_DEPS}"
)

set(checksum_server_slices
	${CMAKE_CURRENT_SOURCE_DIR}/ServerTypes.ice
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
)

set(checksum_server_sources
	${CMAKE_CURRENT_SOURCE_DIR}/Server.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.h
)

CreateExecutableIceTest(Ice
	"${ice_checksum_includes}"
	${ICE_TEST_OUTPUT_DIR}/checksum 
	"${checksum_server_sources}"
	"${checksum_server_slices}"
	icechecksumserver
	server
	"${ICE_TEST_DEPS}"
)