set(glacier2_override_includes
	${PROJECT_SOURCE_DIR}/cpp/include
	${PROJECT_SOURCE_DIR}/cpp/test/include
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/Glacier2
	${CMAKE_CURRENT_BINARY_DIR}
	${GENERATED_SLICES_DIR}
)

set(override_client_sources
	${CMAKE_CURRENT_SOURCE_DIR}/CallbackI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/CallbackI.h
	${CMAKE_CURRENT_SOURCE_DIR}/Client.cpp
)

CreateExecutableIceTest(Glacier2
	"${glacier2_override_includes}"
	${GLACIER2_TEST_OUTPUT_DIR}/override
	"${override_client_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Callback.ice
	glacier2overrideclient
	client
	"${GLACIER2_TEST_DEPS}"
)

set(override_server_sources
	${CMAKE_CURRENT_SOURCE_DIR}/CallbackI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/CallbackI.h
	${CMAKE_CURRENT_SOURCE_DIR}/Server.cpp
)

CreateExecutableIceTest(Glacier2
	"${glacier2_override_includes}"
	${GLACIER2_TEST_OUTPUT_DIR}/override
	"${override_server_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Callback.ice
	glacier2overrideserver
	server
	"${GLACIER2_TEST_DEPS}"
)
