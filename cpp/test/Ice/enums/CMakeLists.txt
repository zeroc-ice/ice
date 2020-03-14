set(ice_enums_includes
	${PROJECT_SOURCE_DIR}/cpp/include
	${PROJECT_SOURCE_DIR}/cpp/test/include
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/Ice
	${CMAKE_CURRENT_BINARY_DIR}
	${GENERATED_SLICES_DIR}
)

set(enums_client_sources
	${CMAKE_CURRENT_SOURCE_DIR}/AllTests.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Client.cpp
)

CreateExecutableIceTest(Ice
	"${ice_enums_includes}"
	${ICE_TEST_OUTPUT_DIR}/enums 
	"${enums_client_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	iceenumsclient
	client 
	"${ICE_TEST_DEPS}"
)

set(enums_server_sources
	${CMAKE_CURRENT_SOURCE_DIR}/Server.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.h
)

CreateExecutableIceTest(Ice
	"${ice_enums_includes}"
	${ICE_TEST_OUTPUT_DIR}/enums 
	"${enums_server_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	iceenumsserver
	server
	"${ICE_TEST_DEPS}"
)
