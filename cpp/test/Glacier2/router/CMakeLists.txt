set(glacier2_router_includes
	${PROJECT_SOURCE_DIR}/cpp/include
	${PROJECT_SOURCE_DIR}/cpp/test/include
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/Glacier2
	${CMAKE_CURRENT_BINARY_DIR}
	${GENERATED_SLICES_DIR}
)

set(router_client_sources
	${CMAKE_CURRENT_SOURCE_DIR}/CallbackI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/CallbackI.h
	${CMAKE_CURRENT_SOURCE_DIR}/Client.cpp
)

CreateExecutableIceTest(Glacier2
	"${glacier2_router_includes}"
	${GLACIER2_TEST_OUTPUT_DIR}/router
	"${router_client_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Callback.ice
	glacier2routerclient
	client
	"${GLACIER2_TEST_DEPS}"
)

set(router_server_sources
	${CMAKE_CURRENT_SOURCE_DIR}/CallbackI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/CallbackI.h
	${CMAKE_CURRENT_SOURCE_DIR}/Server.cpp
)

CreateExecutableIceTest(Glacier2
	"${glacier2_router_includes}"
	${GLACIER2_TEST_OUTPUT_DIR}/router
	"${router_server_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Callback.ice
	glacier2routerserver
	server
	"${GLACIER2_TEST_DEPS}"
)