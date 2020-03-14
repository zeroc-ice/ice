set(ice_defvalue_includes
	${PROJECT_SOURCE_DIR}/cpp/include
	${PROJECT_SOURCE_DIR}/cpp/test/include
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/Ice
	${CMAKE_CURRENT_BINARY_DIR}
	${GENERATED_SLICES_DIR}
)

set(defaultValue_client_sources
	${CMAKE_CURRENT_SOURCE_DIR}/Alltests.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Client.cpp
)

CreateExecutableIceTest(Ice
	"${ice_defvalue_includes}"
	${ICE_TEST_OUTPUT_DIR}/defaultValue
	"${defaultValue_client_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	icedefvalueclient
	client
	"${ICE_TEST_DEPS}"
)
