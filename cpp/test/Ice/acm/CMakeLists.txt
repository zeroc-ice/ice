set(ice_acm_includes
	${PROJECT_SOURCE_DIR}/cpp/include
	${PROJECT_SOURCE_DIR}/cpp/test/include
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/Ice
	${CMAKE_CURRENT_BINARY_DIR}
	${GENERATED_SLICES_DIR}
)

set(acm_client_sources
	${CMAKE_CURRENT_SOURCE_DIR}/AllTests.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Client.cpp
)

CreateExecutableIceTest(Ice
	"${ice_acm_includes}"
	${ICE_TEST_OUTPUT_DIR}/acm 
	"${acm_client_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	iceacmclient
	client 
	"${ICE_TEST_DEPS}"
)

set(acm_server_sources
	${CMAKE_CURRENT_SOURCE_DIR}/Server.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.h
)

CreateExecutableIceTest(Ice
	"${ice_acm_includes}"
	${ICE_TEST_OUTPUT_DIR}/acm 
	"${acm_server_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	iceacmserver
	server
	"${ICE_TEST_DEPS}"
)
