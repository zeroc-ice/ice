set(slice_parser_includes
	${PROJECT_SOURCE_DIR}/cpp/include
	${PROJECT_SOURCE_DIR}/cpp/src
	${PROJECT_SOURCE_DIR}/cpp/test/include
	${CMAKE_CURRENT_BINARY_DIR}/Slice
	${CMAKE_CURRENT_BINARY_DIR}
	${GENERATED_SLICES_DIR}
)

set(parser_slices
	${CMAKE_CURRENT_SOURCE_DIR}/CircularA.ice
	${CMAKE_CURRENT_SOURCE_DIR}/CircularB.ice
)

CreateLibraryInternalIceTest(Slice
	"${slice_parser_includes}"
	${SLICE_TEST_OUTPUT_DIR}/parser
	""
	"${parser_slices}"
	sliceparserclient
	client
	"${SLICE_TEST_DEPS}"
)
