set(slice_macros_includes
	${PROJECT_SOURCE_DIR}/cpp/include
	${PROJECT_SOURCE_DIR}/cpp/test/include
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/Slice
	${CMAKE_CURRENT_BINARY_DIR}
	${GENERATED_SLICES_DIR}
)

CreateExecutableIceTest(Slice
	"${slice_macros_includes}"
	${SLICE_TEST_OUTPUT_DIR}/macros
	${CMAKE_CURRENT_SOURCE_DIR}/Client.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	slicemacrosclient
	client
	"${SLICE_TEST_DEPS}"
)
