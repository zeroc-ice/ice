set(iceutil_timer_includes
	${PROJECT_SOURCE_DIR}/cpp/include
	${PROJECT_SOURCE_DIR}/cpp/src
	${PROJECT_SOURCE_DIR}/cpp/test/include
	${GENERATED_SLICES_DIR}
)

CreateExecutableTest(IceUtil
	"${iceutil_timer_includes}"
	${ICEUTIL_TEST_OUTPUT_DIR}/timer
	${CMAKE_CURRENT_SOURCE_DIR}/Client.cpp
	iceutiltimerclient
	client
	"${ICEUTIL_TEST_DEPS}"
)
