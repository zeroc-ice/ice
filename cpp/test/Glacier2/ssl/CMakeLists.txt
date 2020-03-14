set(glacier2_ssl_includes
	${PROJECT_SOURCE_DIR}/cpp/include
	${PROJECT_SOURCE_DIR}/cpp/test/include
	${GENERATED_SLICES_DIR}
)

CreateExecutableTest(Glacier2
	"${glacier2_ssl_includes}"
	${GLACIER2_TEST_OUTPUT_DIR}/ssl
	${CMAKE_CURRENT_SOURCE_DIR}/Client.cpp
	glacier2sslclient
	client
	"${GLACIER2_TEST_DEPS}"
)

CreateExecutableTest(Glacier2
	"${glacier2_ssl_includes}"
	${GLACIER2_TEST_OUTPUT_DIR}/ssl
	${CMAKE_CURRENT_SOURCE_DIR}/Server.cpp
	glacier2sslserver
	server
	"${GLACIER2_TEST_DEPS}"
)
