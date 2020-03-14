set(ICEUTIL_TEST_DEPS
	ice
	testcommon
)

set(ICEUTIL_TEST_OUTPUT_DIR ${PROJECT_BINARY_DIR}/cpp/test/IceStorm)

add_subdirectory(ctrlCHandler)
add_subdirectory(inputUtil)
add_subdirectory(priority)
add_subdirectory(sha1)
add_subdirectory(stackTrace)
add_subdirectory(thread)
add_subdirectory(timer)
add_subdirectory(unicode)
add_subdirectory(uuid)
