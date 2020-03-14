set(glacier2_sessionhlpr_includes
	${PROJECT_SOURCE_DIR}/cpp/include
	${PROJECT_SOURCE_DIR}/cpp/test/include
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/Glacier2
	${CMAKE_CURRENT_BINARY_DIR}
	${GENERATED_SLICES_DIR}
)

CreateExecutableIceTest(Glacier2
	"${glacier2_sessionhlpr_includes}"
	${GLACIER2_TEST_OUTPUT_DIR}/sessionHelper
	${CMAKE_CURRENT_SOURCE_DIR}/Client.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Callback.ice
	glacier2sessionhlprclient
	client
	"${GLACIER2_TEST_DEPS}"
)

CreateExecutableIceTest(Glacier2
	"${glacier2_sessionhlpr_includes}"
	${GLACIER2_TEST_OUTPUT_DIR}/sessionHelper
	${CMAKE_CURRENT_SOURCE_DIR}/Server.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Callback.ice
	glacier2sessionhlprserver
	server
	"${GLACIER2_TEST_DEPS}"
)
