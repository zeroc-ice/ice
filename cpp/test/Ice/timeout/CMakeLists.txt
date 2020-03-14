set(ice_timeout_includes
	${PROJECT_SOURCE_DIR}/cpp/include
	${PROJECT_SOURCE_DIR}/cpp/test/include
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/Ice
	${CMAKE_CURRENT_BINARY_DIR}
	${GENERATED_SLICES_DIR}
)

set(timeout_client_sources
	${CMAKE_CURRENT_SOURCE_DIR}/AllTests.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Client.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.h
)

CreateExecutableIceTest(Ice
	"${ice_timeout_includes}"
	${ICE_TEST_OUTPUT_DIR}/timeout 
	"${timeout_client_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	icetimeoutclient
	client 
	"${ICE_TEST_DEPS}"
)

set(timeout_server_sources
	${CMAKE_CURRENT_SOURCE_DIR}/Server.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.h
)

CreateExecutableIceTest(Ice
	"${ice_timeout_includes}"
	${ICE_TEST_OUTPUT_DIR}/timeout 
	"${timeout_server_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	icetimeoutserver
	server
	"${ICE_TEST_DEPS}"
)
