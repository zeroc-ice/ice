set(slice_utf8bom_includes
	${PROJECT_SOURCE_DIR}/cpp/include
	${PROJECT_SOURCE_DIR}/cpp/test/include
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/Slice
	${CMAKE_CURRENT_BINARY_DIR}
	${GENERATED_SLICES_DIR}
)

CreateLibraryIceTest(Slice
	"${slice_utf8bom_includes}"
	${SLICE_TEST_OUTPUT_DIR}/utf8BOM
	""
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	sliceutf8BOMclient
	client
	"${SLICE_TEST_DEPS}"
)
