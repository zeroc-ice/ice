set(icegrid_admin_includes
	${PROJECT_SOURCE_DIR}/cpp/include
	${PROJECT_SOURCE_DIR}/cpp/src
	${PROJECT_SOURCE_DIR}/cpp/test/include
	${CMAKE_CURRENT_SOURCE_DIR}
	${GENERATED_SLICES_DIR}
)

CreateExecutableTest(IceGrid
	"${icegrid_admin_includes}"
	${ICEGRID_TEST_OUTPUT_DIR}/admin
	${CMAKE_CURRENT_SOURCE_DIR}/Server.cpp
	icegridadminserver
	server
	"${ICEGRID_TEST_DEPS}"
)
