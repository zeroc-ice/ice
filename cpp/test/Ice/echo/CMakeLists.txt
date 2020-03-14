set(ice_echo_includes
	${PROJECT_SOURCE_DIR}/cpp/include
	${PROJECT_SOURCE_DIR}/cpp/test/include
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/Ice
	${CMAKE_CURRENT_BINARY_DIR}
	${GENERATED_SLICES_DIR}
)

set(echo_server_sources
	${CMAKE_CURRENT_SOURCE_DIR}/BlobjectI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/BlobjectI.h
	${CMAKE_CURRENT_SOURCE_DIR}/Server.cpp
)

CreateExecutableIceTest(Ice
	"${ice_echo_includes}"
	${ICE_TEST_OUTPUT_DIR}/echo 
	"${echo_server_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	iceechoserver
	server
	"${ICE_TEST_DEPS}"
)
