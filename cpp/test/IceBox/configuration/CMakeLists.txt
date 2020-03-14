set(icebox_config_includes
	${PROJECT_SOURCE_DIR}/cpp/include
	${PROJECT_SOURCE_DIR}/cpp/test/include
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/IceBox
	${CMAKE_CURRENT_BINARY_DIR}
	${GENERATED_SLICES_DIR}
)

set(configuration_client_sources
	${CMAKE_CURRENT_SOURCE_DIR}/AllTests.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Client.cpp
)

CreateExecutableIceTest(IceBox
	"${icebox_config_includes}"
	${ICEBOX_TEST_OUTPUT_DIR}/configuration
	"${configuration_client_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	iceboxconfigurationclient
	client
	"${ICEBOX_TEST_DEPS}"
)

set(configuration_testsvc_sources
	${CMAKE_CURRENT_SOURCE_DIR}/Service.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.h
)

CreateLibraryIceTest(IceBox
	"${icebox_config_includes}"
	${ICEBOX_TEST_OUTPUT_DIR}/configuration
	"${configuration_testsvc_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	iceboxconfigurationtestsvc
	testservice
	"${ICEBOX_TEST_SERVICE_DEPS}"
)
