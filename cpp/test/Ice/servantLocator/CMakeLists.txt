set(ice_servlocator_includes
	${PROJECT_SOURCE_DIR}/cpp/include
	${PROJECT_SOURCE_DIR}/cpp/test/include
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/Ice
	${CMAKE_CURRENT_BINARY_DIR}
	${GENERATED_SLICES_DIR}
)

set(servantLocator_client_sources
	${CMAKE_CURRENT_SOURCE_DIR}/AllTests.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Client.cpp
)

CreateExecutableIceTest(Ice
	"${ice_servlocator_includes}"
	${ICE_TEST_OUTPUT_DIR}/servantLocator 
	"${servantLocator_client_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	iceservlocatorclient
	client 
	"${ICE_TEST_DEPS}"
)

set(servantLocator_collocated_sources
	${CMAKE_CURRENT_SOURCE_DIR}/AllTests.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Collocated.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/ServantLocatorI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/ServantLocatorI.h
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.h
)

CreateExecutableIceTest(Ice
	"${ice_servlocator_includes}"
	${ICE_TEST_OUTPUT_DIR}/servantLocator 
	"${servantLocator_collocated_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	iceservlocatorcollocated
	collocated 
	"${ICE_TEST_DEPS}"
)

set(servantLocator_server_sources
	${CMAKE_CURRENT_SOURCE_DIR}/ServantLocatorI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/ServantLocatorI.h
	${CMAKE_CURRENT_SOURCE_DIR}/Server.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.h
)

CreateExecutableIceTest(Ice
	"${ice_servlocator_includes}"
	${ICE_TEST_OUTPUT_DIR}/servantLocator 
	"${servantLocator_server_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	iceservlocatorserver
	server 
	"${ICE_TEST_DEPS}"
)

set(servantLocator_serveramd_sources
	${CMAKE_CURRENT_SOURCE_DIR}/ServantLocatorI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/ServantLocatorI.h
	${CMAKE_CURRENT_SOURCE_DIR}/ServerAMD.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestAMDI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestAMDI.h
)

CreateAMDExecutableIceTest(Ice
	"${ice_servlocator_includes}"
	${ICE_TEST_OUTPUT_DIR}/servantLocator 
	"${servantLocator_serveramd_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/TestAMD.ice
	iceservlocatorserveramd
	serveramd
	"${ICE_TEST_DEPS}"
)
