set(icestorm_repstress_includes
	${PROJECT_SOURCE_DIR}/cpp/include
	${PROJECT_SOURCE_DIR}/cpp/test/include
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/IceStorm
	${CMAKE_CURRENT_BINARY_DIR}
	${GENERATED_SLICES_DIR}
)

set(repstress_slices
	${CMAKE_CURRENT_SOURCE_DIR}/Controller.ice
	${CMAKE_CURRENT_SOURCE_DIR}/Single.ice
)

CreateExecutableIceTest(IceStorm
	"${icestorm_repstress_includes}"
	${ICESTORM_TEST_OUTPUT_DIR}/repstress
	${CMAKE_CURRENT_SOURCE_DIR}/Control.cpp
	"${repstress_slices}"
	icestormrepstresscontrol
	control
	"${ICESTORM_TEST_DEPS}"
)

CreateExecutableIceTest(IceStorm
	"${icestorm_repstress_includes}"
	${ICESTORM_TEST_OUTPUT_DIR}/repstress
	${CMAKE_CURRENT_SOURCE_DIR}/Publisher.cpp
	"${repstress_slices}"
	icestormrepstresspublisher
	publisher
	"${ICESTORM_TEST_DEPS}"
)

CreateExecutableIceTest(IceStorm
	"${icestorm_repstress_includes}"
	${ICESTORM_TEST_OUTPUT_DIR}/repstress
	${CMAKE_CURRENT_SOURCE_DIR}/Subscriber.cpp
	"${repstress_slices}"
	icestormrepstresssubscriber
	subscriber
	"${ICESTORM_TEST_DEPS}"
)