set(ice_location_includes
	${PROJECT_SOURCE_DIR}/cpp/include
	${PROJECT_SOURCE_DIR}/cpp/test/include
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/Ice
	${CMAKE_CURRENT_BINARY_DIR}
	${GENERATED_SLICES_DIR}
)

set(location_client_sources
	${CMAKE_CURRENT_SOURCE_DIR}/AllTests.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Client.cpp
)

CreateExecutableIceTest(Ice
	"${ice_location_includes}"
	${ICE_TEST_OUTPUT_DIR}/location 
	"${location_client_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	icelocationclient
	client 
	"${ICE_TEST_DEPS}"
)

set(location_server_sources
	${CMAKE_CURRENT_SOURCE_DIR}/Server.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/ServerLocator.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/ServerLocator.h
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.h
)

CreateExecutableIceTest(Ice
	"${ice_location_includes}"
	${ICE_TEST_OUTPUT_DIR}/location 
	"${location_server_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	icelocationserver
	server
	"${ICE_TEST_DEPS}"
)
