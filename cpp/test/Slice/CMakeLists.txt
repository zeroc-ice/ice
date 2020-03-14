set(SLICE_TEST_DEPS
	ice
	testcommon
)

set(SLICE_TEST_OUTPUT_DIR ${PROJECT_BINARY_DIR}/cpp/test/Slice)

add_subdirectory(escape)
add_subdirectory(macros)
add_subdirectory(parser)
add_subdirectory(structure)
add_subdirectory(utf8BOM)
