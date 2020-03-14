set(ice_dispatcher_includes
	${PROJECT_SOURCE_DIR}/cpp/include
	${PROJECT_SOURCE_DIR}/cpp/test/include
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/Ice
	${CMAKE_CURRENT_BINARY_DIR}
	${GENERATED_SLICES_DIR}
)

set(dispatcher_client_sources
	${CMAKE_CURRENT_SOURCE_DIR}/AllTests.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Client.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Dispatcher.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Dispatcher.h
)

CreateExecutableIceTest(Ice
	"${ice_dispatcher_includes}"
	${ICE_TEST_OUTPUT_DIR}/dispatcher 
	"${dispatcher_client_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	icedispatcherclient
	client 
	"${ICE_TEST_DEPS}"
)

set(dispatcher_collocated_sources
	${CMAKE_CURRENT_SOURCE_DIR}/AllTests.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Collocated.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Dispatcher.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Dispatcher.h
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.h
)

CreateExecutableIceTest(Ice
	"${ice_dispatcher_includes}"
	${ICE_TEST_OUTPUT_DIR}/dispatcher 
	"${dispatcher_collocated_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	icedispatchercollocated
	collocated 
	"${ICE_TEST_DEPS}"
)

set(dispatcher_server_sources
	${CMAKE_CURRENT_SOURCE_DIR}/Dispatcher.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Dispatcher.h
	${CMAKE_CURRENT_SOURCE_DIR}/Server.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.h
)

CreateExecutableIceTest(Ice
	"${ice_dispatcher_includes}"
	${ICE_TEST_OUTPUT_DIR}/dispatcher 
	"${dispatcher_server_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	icedispatcherserver
	server 
	"${ICE_TEST_DEPS}"
)
