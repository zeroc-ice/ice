set(ice_properties_includes
	${PROJECT_SOURCE_DIR}/cpp/include
	${PROJECT_SOURCE_DIR}/cpp/src
	${PROJECT_SOURCE_DIR}/cpp/test/include
	${CMAKE_CURRENT_SOURCE_DIR}
	${GENERATED_SLICES_DIR}
)

CreateExecutableTest(Ice
	"${ice_properties_includes}"
	${ICE_TEST_OUTPUT_DIR}/properties
	${CMAKE_CURRENT_SOURCE_DIR}/Client.cpp
	icepropertiesclient
	client
	"${ICE_TEST_DEPS}"
)
