set(ice_optional_includes
	${PROJECT_SOURCE_DIR}/cpp/include
	${PROJECT_SOURCE_DIR}/cpp/test/include
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/Ice
	${CMAKE_CURRENT_BINARY_DIR}
	${GENERATED_SLICES_DIR}
)

set(optional_client_sources
	${CMAKE_CURRENT_SOURCE_DIR}/AllTests.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Client.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.h
)

CreateExecutableIceTest(Ice
	"${ice_optional_includes}"
	${ICE_TEST_OUTPUT_DIR}/optional 
	"${optional_client_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	iceoptionalclient
	client 
	"${ICE_TEST_DEPS}"
)

set(optional_server_sources
	${CMAKE_CURRENT_SOURCE_DIR}/Server.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.h
)

CreateExecutableIceTest(Ice
	"${ice_optional_includes}"
	${ICE_TEST_OUTPUT_DIR}/optional 
	"${optional_server_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	iceoptionalserver
	server
	"${ICE_TEST_DEPS}"
)

set(optional_serveramd_sources
	${CMAKE_CURRENT_SOURCE_DIR}/ServerAMD.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestAMDI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestAMDI.h
)

CreateAMDExecutableIceTest(Ice
	"${ice_optional_includes}"
	${ICE_TEST_OUTPUT_DIR}/optional 
	"${optional_serveramd_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/TestAMD.ice
	iceoptionalserveramd
	serveramd
	"${ICE_TEST_DEPS}"
)
