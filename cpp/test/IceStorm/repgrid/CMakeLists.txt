set(icestorm_repgrid_includes
	${PROJECT_SOURCE_DIR}/cpp/include
	${PROJECT_SOURCE_DIR}/cpp/test/include
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/IceStorm
	${CMAKE_CURRENT_BINARY_DIR}
	${GENERATED_SLICES_DIR}
)

CreateExecutableIceTest(IceStorm
	"${icestorm_repgrid_includes}"
	${ICESTORM_TEST_OUTPUT_DIR}/repgrid
	${CMAKE_CURRENT_SOURCE_DIR}/Client.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Single.ice
	icestormrepgridclient
	client
	"${ICESTORM_TEST_DEPS}"
)
