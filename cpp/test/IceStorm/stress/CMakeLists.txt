set(icestorm_stress_includes
	${PROJECT_SOURCE_DIR}/cpp/include
	${PROJECT_SOURCE_DIR}/cpp/test/include
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/IceStorm
	${CMAKE_CURRENT_BINARY_DIR}
	${GENERATED_SLICES_DIR}
)

CreateExecutableIceTest(IceStorm
	"${icestorm_stress_includes}"
	${ICESTORM_TEST_OUTPUT_DIR}/stress
	${CMAKE_CURRENT_SOURCE_DIR}/Publisher.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Event.ice
	icestormstresspublisher
	publisher
	"${ICESTORM_TEST_DEPS}"
)

CreateExecutableIceTest(IceStorm
	"${icestorm_stress_includes}"
	${ICESTORM_TEST_OUTPUT_DIR}/stress
	${CMAKE_CURRENT_SOURCE_DIR}/Subscriber.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Event.ice
	icestormstresssubscriber
	subscriber
	"${ICESTORM_TEST_DEPS}"
)
