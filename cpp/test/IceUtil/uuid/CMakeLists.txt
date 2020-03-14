set(iceutil_uuid_includes
	${PROJECT_SOURCE_DIR}/cpp/include
	${PROJECT_SOURCE_DIR}/cpp/src
	${PROJECT_SOURCE_DIR}/cpp/test/include
	${GENERATED_SLICES_DIR}
)

CreateExecutableTest(IceUtil
	"${iceutil_uuid_includes}"
	${ICEUTIL_TEST_OUTPUT_DIR}/uuid
	${CMAKE_CURRENT_SOURCE_DIR}/Client.cpp
	iceutiluuidclient
	client
	"${ICEUTIL_TEST_DEPS}"
)
