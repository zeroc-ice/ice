set(slice_escape_includes
	${PROJECT_SOURCE_DIR}/cpp/include
	${PROJECT_SOURCE_DIR}/cpp/src
	${PROJECT_SOURCE_DIR}/cpp/test/include
	${CMAKE_CURRENT_BINARY_DIR}/Slice
	${CMAKE_CURRENT_BINARY_DIR}
	${GENERATED_SLICES_DIR}
)

set(escape_slices
	${CMAKE_CURRENT_SOURCE_DIR}/Clash.ice
	${CMAKE_CURRENT_SOURCE_DIR}/Key.ice
)

CreateExecutableIceTest(Slice
	"${slice_escape_includes}"
	${SLICE_TEST_OUTPUT_DIR}/escape
	${CMAKE_CURRENT_SOURCE_DIR}/Client.cpp
	"${escape_slices}"
	sliceescapeclient
	client
	"${SLICE_TEST_DEPS}"
)