set(ice_hash_includes
	${PROJECT_SOURCE_DIR}/cpp/include
	${PROJECT_SOURCE_DIR}/cpp/test/include
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/Ice
	${CMAKE_CURRENT_BINARY_DIR}
	${GENERATED_SLICES_DIR}
)

CreateExecutableIceTest(Ice
	"${ice_hash_includes}"
	${ICE_TEST_OUTPUT_DIR}/hash
	${CMAKE_CURRENT_SOURCE_DIR}/Client.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	icehashclient
	client
	"${ICE_TEST_DEPS}"
)
