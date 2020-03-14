set(ice_hold_includes
	${PROJECT_SOURCE_DIR}/cpp/include
	${PROJECT_SOURCE_DIR}/cpp/test/include
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/Ice
	${CMAKE_CURRENT_BINARY_DIR}
	${GENERATED_SLICES_DIR}
)

set(hold_client_sources
	${CMAKE_CURRENT_SOURCE_DIR}/AllTests.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Client.cpp
)

CreateExecutableIceTest(Ice
	"${ice_hold_includes}"
	${ICE_TEST_OUTPUT_DIR}/hold
	"${hold_client_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	iceholdclient
	client 
	"${ICE_TEST_DEPS}"
)

set(hold_server_sources
	${CMAKE_CURRENT_SOURCE_DIR}/Server.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.h
)

CreateExecutableIceTest(Ice
	"${ice_hold_includes}"
	${ICE_TEST_OUTPUT_DIR}/hold 
	"${hold_server_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	iceholdserver
	server
	"${ICE_TEST_DEPS}"
)
