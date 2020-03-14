set(icegrid_activation_includes
	${PROJECT_SOURCE_DIR}/cpp/include
	${PROJECT_SOURCE_DIR}/cpp/test/include
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/IceGrid
	${CMAKE_CURRENT_BINARY_DIR}
	${GENERATED_SLICES_DIR}
)

set(activation_client_sources
	${CMAKE_CURRENT_SOURCE_DIR}/AllTests.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Client.cpp
)

CreateExecutableIceTest(IceGrid
	"${icegrid_activation_includes}"
	${ICEGRID_TEST_OUTPUT_DIR}/activation
	"${activation_client_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	icegridactivationclient
	client
	"${ICEGRID_TEST_DEPS}"
)

set(activation_server_sources
	${CMAKE_CURRENT_SOURCE_DIR}/Server.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.h
)

CreateExecutableIceTest(IceGrid
	"${icegrid_activation_includes}"
	${ICEGRID_TEST_OUTPUT_DIR}/activation
	"${activation_server_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	icegridactivationserver
	server
	"${ICEGRID_TEST_DEPS}"
)
