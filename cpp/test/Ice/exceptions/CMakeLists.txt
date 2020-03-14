set(ice_exceptions_includes
	${PROJECT_SOURCE_DIR}/cpp/include
	${PROJECT_SOURCE_DIR}/cpp/test/include
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/Ice
	${CMAKE_CURRENT_BINARY_DIR}
	${GENERATED_SLICES_DIR}
)

set(exceptions_client_sources
	${CMAKE_CURRENT_SOURCE_DIR}/AllTests.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Client.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/ExceptionsI.cpp
)

CreateExecutableIceTest(Ice
	"${ice_exceptions_includes}"
	${ICE_TEST_OUTPUT_DIR}/exceptions 
	"${exceptions_client_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	iceexceptionsclient
	client 
	"${ICE_TEST_DEPS}"
)

set(exceptions_collocated_sources
	${CMAKE_CURRENT_SOURCE_DIR}/AllTests.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Collocated.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/ExceptionsI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.h
)

CreateExecutableIceTest(Ice
	"${ice_exceptions_includes}"
	${ICE_TEST_OUTPUT_DIR}/exceptions 
	"${exceptions_collocated_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	iceexceptionscollocated
	collocated 
	"${ICE_TEST_DEPS}"
)

set(exceptions_server_sources
	${CMAKE_CURRENT_SOURCE_DIR}/ExceptionsI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Server.cpp	
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.h
)

CreateExecutableIceTest(Ice
	"${ice_exceptions_includes}"
	${ICE_TEST_OUTPUT_DIR}/exceptions 
	"${exceptions_server_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	iceexceptionsserver
	server 
	"${ICE_TEST_DEPS}"
)

set(exceptions_serveramd_sources
	${CMAKE_CURRENT_SOURCE_DIR}/ExceptionsI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/ServerAMD.cpp	
	${CMAKE_CURRENT_SOURCE_DIR}/TestAMDI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestAMDI.h
)

CreateAMDExecutableIceTest(Ice
	"${ice_exceptions_includes}"
	${ICE_TEST_OUTPUT_DIR}/exceptions 
	"${exceptions_serveramd_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/TestAMD.ice
	iceexceptionsserveramd
	serveramd 
	"${ICE_TEST_DEPS}"
)
