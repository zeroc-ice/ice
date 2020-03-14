set(iceutil_stacktrace_includes
	${PROJECT_SOURCE_DIR}/cpp/include
	${PROJECT_SOURCE_DIR}/cpp/src
	${PROJECT_SOURCE_DIR}/cpp/test/include
	${GENERATED_SLICES_DIR}
)

CreateExecutableTest(IceUtil
	"${iceutil_stacktrace_includes}"
	${ICEUTIL_TEST_OUTPUT_DIR}/stacktrace
	${CMAKE_CURRENT_SOURCE_DIR}/Client.cpp
	iceutilstacktraceclient
	client
	"${ICEUTIL_TEST_DEPS}"
)
