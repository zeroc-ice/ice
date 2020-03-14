set(ice_invoke_includes
	${PROJECT_SOURCE_DIR}/cpp/include
	${PROJECT_SOURCE_DIR}/cpp/test/include
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/Ice
	${CMAKE_CURRENT_BINARY_DIR}
	${GENERATED_SLICES_DIR}
)

set(invoke_client_sources
	${CMAKE_CURRENT_SOURCE_DIR}/AllTests.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Client.cpp
)

CreateExecutableIceTest(Ice
	"${ice_invoke_includes}"
	${ICE_TEST_OUTPUT_DIR}/invoke 
	"${invoke_client_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	iceinvokeclient
	client 
	"${ICE_TEST_DEPS}"
)

set(invoke_server_sources
	${CMAKE_CURRENT_SOURCE_DIR}/BlobjectI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/BlobjectI.h
	${CMAKE_CURRENT_SOURCE_DIR}/Server.cpp
)

CreateExecutableIceTest(Ice
	"${ice_invoke_includes}"
	${ICE_TEST_OUTPUT_DIR}/invoke 
	"${invoke_server_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	iceinvokeserver
	server
	"${ICE_TEST_DEPS}"
)
