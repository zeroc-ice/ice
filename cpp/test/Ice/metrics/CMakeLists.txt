set(ice_metrics_includes
	${PROJECT_SOURCE_DIR}/cpp/include
	${PROJECT_SOURCE_DIR}/cpp/test/include
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/Ice
	${CMAKE_CURRENT_BINARY_DIR}
	${GENERATED_SLICES_DIR}
)

set(metrics_client_sources
	${CMAKE_CURRENT_SOURCE_DIR}/AllTests.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Client.cpp
)

CreateExecutableIceTest(Ice
	"${ice_metrics_includes}"
	${ICE_TEST_OUTPUT_DIR}/metrics 
	"${metrics_client_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	icemetricsclient
	client 
	"${ICE_TEST_DEPS}"
)

set(metrics_collocated_sources
	${CMAKE_CURRENT_SOURCE_DIR}/AllTests.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Collocated.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.h
)

CreateExecutableIceTest(Ice
	"${ice_metrics_includes}"
	${ICE_TEST_OUTPUT_DIR}/metrics 
	"${metrics_collocated_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	icemetricscollocated
	collocated 
	"${ICE_TEST_DEPS}"
)

set(metrics_server_sources
	${CMAKE_CURRENT_SOURCE_DIR}/Server.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.h
)

CreateExecutableIceTest(Ice
	"${ice_metrics_includes}"
	${ICE_TEST_OUTPUT_DIR}/metrics 
	"${metrics_server_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	icemetricsserver
	server 
	"${ICE_TEST_DEPS}"
)

set(metrics_serveramd_sources
	${CMAKE_CURRENT_SOURCE_DIR}/ServerAMD.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestAMDI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestAMDI.h
)

CreateAMDExecutableIceTest(Ice
	"${ice_metrics_includes}"
	${ICE_TEST_OUTPUT_DIR}/metrics 
	"${metrics_serveramd_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/TestAMD.ice
	icemetricsserveramd
	serveramd
	"${ICE_TEST_DEPS}"
)
