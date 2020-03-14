set(icestorm_persist_includes
	${PROJECT_SOURCE_DIR}/cpp/include
	${PROJECT_SOURCE_DIR}/cpp/test/include
	${GENERATED_SLICES_DIR}
)

CreateExecutableTest(IceStorm
	"${icestorm_persist_includes}"
	${ICESTORM_TEST_OUTPUT_DIR}/persistent
	${CMAKE_CURRENT_SOURCE_DIR}/Client.cpp
	icestormpersistclient
	client
	"${ICESTORM_TEST_DEPS}"
)
