set(icestorm_rep1_includes
	${PROJECT_SOURCE_DIR}/cpp/include
	${PROJECT_SOURCE_DIR}/cpp/test/include
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/IceStorm
	${CMAKE_CURRENT_BINARY_DIR}
	${GENERATED_SLICES_DIR}
)

CreateExecutableIceTest(IceStorm
	"${icestorm_rep1_includes}"
	${ICESTORM_TEST_OUTPUT_DIR}/rep1
	${CMAKE_CURRENT_SOURCE_DIR}/Publisher.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Single.ice
	icestormrep1publisher
	publisher
	"${ICESTORM_TEST_DEPS}"
)

CreateExecutableIceTest(IceStorm
	"${icestorm_rep1_includes}"
	${ICESTORM_TEST_OUTPUT_DIR}/rep1
	${CMAKE_CURRENT_SOURCE_DIR}/Sub.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Single.ice
	icestormrep1sub
	sub
	"${ICESTORM_TEST_DEPS}"
)

CreateExecutableIceTest(IceStorm
	"${icestorm_rep1_includes}"
	${ICESTORM_TEST_OUTPUT_DIR}/rep1
	${CMAKE_CURRENT_SOURCE_DIR}/Subscriber.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Single.ice
	icestormrep1subscriber
	subscriber
	"${ICESTORM_TEST_DEPS}"
)
