set(ice_adptrdeact_includes
	${PROJECT_SOURCE_DIR}/cpp/include
	${PROJECT_SOURCE_DIR}/cpp/test/include
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/Ice
	${CMAKE_CURRENT_BINARY_DIR}
	${GENERATED_SLICES_DIR}
)

set(adapterDeactivation_client_sources
	${CMAKE_CURRENT_SOURCE_DIR}/AllTests.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Client.cpp
)

CreateExecutableIceTest(Ice
	"${ice_adptrdeact_includes}"
	${ICE_TEST_OUTPUT_DIR}/adapterDeactivation 
	"${adapterDeactivation_client_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	iceadapterdeactclient
	client 
	"${ICE_TEST_DEPS}"
)

set(adapterDeactivation_collocated_sources
	${CMAKE_CURRENT_SOURCE_DIR}/AllTests.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Collocated.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/ServantLocatorI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/ServantLocatorI.h
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.h
)

CreateExecutableIceTest(Ice
	"${ice_adptrdeact_includes}"
	${ICE_TEST_OUTPUT_DIR}/adapterDeactivation 
	"${adapterDeactivation_collocated_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	iceadapterdeactcollocated
	collocated 
	"${ICE_TEST_DEPS}"
)

set(adapterDeactivation_server_sources
	${CMAKE_CURRENT_SOURCE_DIR}/Server.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/ServantLocatorI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/ServantLocatorI.h
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.h
)

CreateExecutableIceTest(Ice
	"${ice_adptrdeact_includes}"
	${ICE_TEST_OUTPUT_DIR}/adapterDeactivation 
	"${adapterDeactivation_server_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	iceadapterdeactserver
	server 
	"${ICE_TEST_DEPS}"
)