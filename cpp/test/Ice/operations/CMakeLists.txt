set(ice_operations_includes
	${PROJECT_SOURCE_DIR}/cpp/include
	${PROJECT_SOURCE_DIR}/cpp/test/include
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/Ice
	${CMAKE_CURRENT_BINARY_DIR}
	${GENERATED_SLICES_DIR}
)

set(operations_client_sources
	${CMAKE_CURRENT_SOURCE_DIR}/AllTests.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/BatchOneways.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/BatchOnewaysAMI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Client.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Oneways.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/OnewaysAMI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Twoways.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TwowaysAMI.cpp
)

CreateExecutableIceTest(Ice
	"${ice_operations_includes}"
	${ICE_TEST_OUTPUT_DIR}/operations 
	"${operations_client_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	iceoperationsclient
	client 
	"${ICE_TEST_DEPS}"
)

set(operations_collocated_sources
	${CMAKE_CURRENT_SOURCE_DIR}/AllTests.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/BatchOneways.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/BatchOnewaysAMI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Collocated.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Oneways.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/OnewaysAMI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.h
	${CMAKE_CURRENT_SOURCE_DIR}/Twoways.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TwowaysAMI.cpp
)

CreateExecutableIceTest(Ice
	"${ice_operations_includes}"
	${ICE_TEST_OUTPUT_DIR}/operations 
	"${operations_collocated_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	iceoperationscollocated
	collocated 
	"${ICE_TEST_DEPS}"
)

set(operations_server_sources
	${CMAKE_CURRENT_SOURCE_DIR}/Server.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.h
)

CreateExecutableIceTest(Ice
	"${ice_operations_includes}"
	${ICE_TEST_OUTPUT_DIR}/operations 
	"${operations_server_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	iceoperationsserver
	server 
	"${ICE_TEST_DEPS}"
)

set(operations_serveramd_sources
	${CMAKE_CURRENT_SOURCE_DIR}/ServerAMD.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestAMDI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestAMDI.h
)

CreateAMDExecutableIceTest(Ice
	"${ice_operations_includes}"
	${ICE_TEST_OUTPUT_DIR}/operations 
	"${operations_serveramd_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/TestAMD.ice
	iceoperationsserveramd
	serveramd
	"${ICE_TEST_DEPS}"
)
