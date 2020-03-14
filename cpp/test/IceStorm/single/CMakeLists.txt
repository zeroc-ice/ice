set(icestorm_single_includes
	${PROJECT_SOURCE_DIR}/cpp/include
	${PROJECT_SOURCE_DIR}/cpp/test/include
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/IceStorm
	${CMAKE_CURRENT_BINARY_DIR}
	${GENERATED_SLICES_DIR}
)

CreateExecutableIceTest(IceStorm
	"${icestorm_single_includes}"
	${ICESTORM_TEST_OUTPUT_DIR}/single
	${CMAKE_CURRENT_SOURCE_DIR}/Publisher.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Single.ice
	icestormsinglepublisher
	publisher
	"${ICESTORM_TEST_DEPS}"
)

CreateExecutableIceTest(IceStorm
	"${icestorm_single_includes}"
	${ICESTORM_TEST_OUTPUT_DIR}/single
	${CMAKE_CURRENT_SOURCE_DIR}/Subscriber.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Single.ice
	icestormsinglesubscriber
	subscriber
	"${ICESTORM_TEST_DEPS}"
)
