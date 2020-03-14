set(slicing_ex_includes
	${PROJECT_SOURCE_DIR}/cpp/include
	${PROJECT_SOURCE_DIR}/cpp/test/include
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/Ice
	${CMAKE_CURRENT_BINARY_DIR}
	${GENERATED_SLICES_DIR}
)

set(exceptions_client_slices
	${CMAKE_CURRENT_SOURCE_DIR}/ClientPrivate.ice
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
)

set(exceptions_client_sources
	${CMAKE_CURRENT_SOURCE_DIR}/AllTests.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Client.cpp
)

CreateExecutableInternalIceTest(Ice
	"${slicing_ex_includes}"
	${ICE_TEST_OUTPUT_DIR}/slicing/exceptions 
	"${exceptions_client_sources}"
	"${exceptions_client_slices}"
	iceslicingexclient
	client 
	"${ICE_TEST_DEPS}"
)

set(exceptions_server_slices
	${CMAKE_CURRENT_SOURCE_DIR}/ServerPrivate.ice
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
)

set(exceptions_server_sources
	${CMAKE_CURRENT_SOURCE_DIR}/Server.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.h
)

CreateExecutableInternalIceTest(Ice
	"${slicing_ex_includes}"
	${ICE_TEST_OUTPUT_DIR}/slicing/exceptions 
	"${exceptions_server_sources}"
	"${exceptions_server_slices}"
	iceslicingexserver
	server
	"${ICE_TEST_DEPS}"
)

set(exceptions_serveramd_slices
	${CMAKE_CURRENT_SOURCE_DIR}/ServerPrivateAMD.ice
	${CMAKE_CURRENT_SOURCE_DIR}/TestAMD.ice
)

set(exceptions_serveramd_sources
	${CMAKE_CURRENT_SOURCE_DIR}/ServerAMD.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestAMDI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestAMDI.h
)

CreateExecutableInternalIceTest(Ice
	"${slicing_ex_includes}"
	${ICE_TEST_OUTPUT_DIR}/slicing/exceptions 
	"${exceptions_serveramd_sources}"
	"${exceptions_serveramd_slices}"
	iceslicingexserveramd
	serveramd
	"${ICE_TEST_DEPS}"
)