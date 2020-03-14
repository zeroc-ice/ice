set(glacier2_sessionctrl_includes
	${PROJECT_SOURCE_DIR}/cpp/include
	${PROJECT_SOURCE_DIR}/cpp/test/include
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/SessionControl
	${CMAKE_CURRENT_BINARY_DIR}
	${GENERATED_SLICES_DIR}
)

CreateExecutableIceTest(SessionControl
	"${glacier2_sessionctrl_includes}"
	${GLACIER2_TEST_OUTPUT_DIR}/sessionControl
	${CMAKE_CURRENT_SOURCE_DIR}/Client.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Session.ice
	glacier2sessionctrlclient
	client
	"${GLACIER2_TEST_DEPS}"
)

set(sessionctrl_server_sources
	${CMAKE_CURRENT_SOURCE_DIR}/Server.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/SessionI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/SessionI.h
)

CreateExecutableIceTest(SessionControl
	"${glacier2_sessionctrl_includes}"
	${GLACIER2_TEST_OUTPUT_DIR}/sessionControl
	"${sessionctrl_server_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Session.ice
	glacier2sessionctrlserver
	server
	"${GLACIER2_TEST_DEPS}"
)
