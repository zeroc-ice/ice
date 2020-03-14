set(ice_background_includes
	${PROJECT_SOURCE_DIR}/cpp/include
	${PROJECT_SOURCE_DIR}/cpp/src
	${PROJECT_SOURCE_DIR}/cpp/test/include
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/Ice
	${CMAKE_CURRENT_BINARY_DIR}
	${GENERATED_SLICES_DIR}
)

set(background_client_sources
	${CMAKE_CURRENT_SOURCE_DIR}/AllTests.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Client.cpp
)

set(background_test_deps
	${ICE_TEST_DEPS}
	icebackgroundtesttransport
)

CreateExecutableTest(Ice
	"${ice_background_includes}"
	${ICE_TEST_OUTPUT_DIR}/background 
	"${background_client_sources}"
	icebackgroundclient
	client
	"${background_test_deps}"
)

set(background_testtransport_sources
	${CMAKE_CURRENT_SOURCE_DIR}/Acceptor.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Acceptor.h
	${CMAKE_CURRENT_SOURCE_DIR}/Configuration.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Configuration.h
	${CMAKE_CURRENT_SOURCE_DIR}/Connector.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Connector.h
	${CMAKE_CURRENT_SOURCE_DIR}/EndpointFactory.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/EndpointFactory.h
	${CMAKE_CURRENT_SOURCE_DIR}/EndpointI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/EndpointI.h
	${CMAKE_CURRENT_SOURCE_DIR}/PluginI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/PluginI.h
	${CMAKE_CURRENT_SOURCE_DIR}/Transceiver.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Transceiver.h
)

CreateLibraryIceTest(Ice
	"${ice_background_includes}"
	${ICE_TEST_OUTPUT_DIR}/background 
	"${background_testtransport_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	icebackgroundtesttransport
	testtransport 
	"${ICE_TEST_DEPS}"
)

set(background_server_sources
	${CMAKE_CURRENT_SOURCE_DIR}/Server.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.h
)

CreateExecutableTest(Ice
	"${ice_background_includes}"
	${ICE_TEST_OUTPUT_DIR}/background 
	"${background_server_sources}"
	icebackgroundserver
	server 
	"${background_test_deps}"
)
