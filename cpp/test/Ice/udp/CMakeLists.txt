set(ice_udp_includes
	${PROJECT_SOURCE_DIR}/cpp/include
	${PROJECT_SOURCE_DIR}/cpp/test/include
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/Ice
	${CMAKE_CURRENT_BINARY_DIR}
	${GENERATED_SLICES_DIR}
)

set(udp_client_sources
	${CMAKE_CURRENT_SOURCE_DIR}/AllTests.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Client.cpp
)

CreateExecutableIceTest(Ice
	"${ice_udp_includes}"
	${ICE_TEST_OUTPUT_DIR}/udp 
	"${udp_client_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	iceudpclient
	client 
	"${ICE_TEST_DEPS}"
)

set(udp_server_sources
	${CMAKE_CURRENT_SOURCE_DIR}/Server.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.h
)

CreateExecutableIceTest(Ice
	"${ice_udp_includes}"
	${ICE_TEST_OUTPUT_DIR}/udp 
	"${udp_server_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	iceudpserver
	server
	"${ICE_TEST_DEPS}"
)
