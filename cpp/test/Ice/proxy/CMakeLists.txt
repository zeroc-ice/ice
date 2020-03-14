set(ice_proxy_includes
	${PROJECT_SOURCE_DIR}/cpp/include
	${PROJECT_SOURCE_DIR}/cpp/test/include
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/Ice
	${CMAKE_CURRENT_BINARY_DIR}
	${GENERATED_SLICES_DIR}
)

set(proxy_client_sources
	${CMAKE_CURRENT_SOURCE_DIR}/AllTests.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Client.cpp
)

CreateExecutableIceTest(Ice
	"${ice_proxy_includes}"
	${ICE_TEST_OUTPUT_DIR}/proxy 
	"${proxy_client_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	iceproxyclient
	client 
	"${ICE_TEST_DEPS}"
)

set(proxy_collocated_sources
	${CMAKE_CURRENT_SOURCE_DIR}/AllTests.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Collocated.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.h
)

CreateExecutableIceTest(Ice
	"${ice_proxy_includes}"
	${ICE_TEST_OUTPUT_DIR}/proxy 
	"${proxy_collocated_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	iceproxycollocated
	collocated 
	"${ICE_TEST_DEPS}"
)

set(proxy_server_sources
	${CMAKE_CURRENT_SOURCE_DIR}/Server.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.h
)

CreateExecutableIceTest(Ice
	"${ice_proxy_includes}"
	${ICE_TEST_OUTPUT_DIR}/proxy 
	"${proxy_server_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	iceproxyserver
	server 
	"${ICE_TEST_DEPS}"
)

set(proxy_serveramd_sources
	${CMAKE_CURRENT_SOURCE_DIR}/ServerAMD.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestAMDI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestAMDI.h
)

CreateAMDExecutableIceTest(Ice
	"${ice_proxy_includes}"
	${ICE_TEST_OUTPUT_DIR}/proxy 
	"${proxy_serveramd_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/TestAMD.ice
	iceproxyserveramd
	serveramd 
	"${ICE_TEST_DEPS}"
)
