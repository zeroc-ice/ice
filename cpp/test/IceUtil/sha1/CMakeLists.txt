set(iceutil_sha1_includes
	${PROJECT_SOURCE_DIR}/cpp/include
	${PROJECT_SOURCE_DIR}/cpp/src
	${PROJECT_SOURCE_DIR}/cpp/test/include
	${GENERATED_SLICES_DIR}
)

CreateExecutableTest(IceUtil
	"${iceutil_sha1_includes}"
	${ICEUTIL_TEST_OUTPUT_DIR}/sha1
	${CMAKE_CURRENT_SOURCE_DIR}/Client.cpp
	iceutilsha1client
	client
	"${ICEUTIL_TEST_DEPS}"
)
