set(ice_defservant_includes
	${PROJECT_SOURCE_DIR}/cpp/include
	${PROJECT_SOURCE_DIR}/cpp/test/include
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/Ice
	${CMAKE_CURRENT_BINARY_DIR}
	${GENERATED_SLICES_DIR}
)

set(defaultServant_client_sources
	${CMAKE_CURRENT_SOURCE_DIR}/Alltests.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Client.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.h
)

CreateExecutableIceTest(Ice
	"${ice_defservant_includes}"
	${ICE_TEST_OUTPUT_DIR}/defaultServant
	"${defaultServant_client_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	icedefservantclient
	client
	"${ICE_TEST_DEPS}"
)
