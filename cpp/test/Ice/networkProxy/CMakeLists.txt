set(ice_netproxy_includes
	${PROJECT_SOURCE_DIR}/cpp/include
	${PROJECT_SOURCE_DIR}/cpp/test/include
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/Ice
	${CMAKE_CURRENT_BINARY_DIR}
	${GENERATED_SLICES_DIR}
)

set(networkProxy_client_sources
	${CMAKE_CURRENT_SOURCE_DIR}/AllTests.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Client.cpp
)

CreateExecutableIceTest(Ice
	"${ice_netproxy_includes}"
	${ICE_TEST_OUTPUT_DIR}/networkProxy 
	"${networkProxy_client_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	icenetproxyclient
	client 
	"${ICE_TEST_DEPS}"
)

CreateExecutableIceTest(Ice
	"${ice_netproxy_includes}"
	${ICE_TEST_OUTPUT_DIR}/networkProxy 
	${CMAKE_CURRENT_SOURCE_DIR}/Server.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	icenetproxyserver
	server
	"${ICE_TEST_DEPS}"
)
