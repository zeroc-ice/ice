set(ice_strconverter_includes
	${PROJECT_SOURCE_DIR}/cpp/include
	${PROJECT_SOURCE_DIR}/cpp/test/include
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/Ice
	${CMAKE_CURRENT_BINARY_DIR}
	${GENERATED_SLICES_DIR}
)

CreateExecutableIceTest(Ice
	"${ice_strconverter_includes}"
	${ICE_TEST_OUTPUT_DIR}/stringConverter
	${CMAKE_CURRENT_SOURCE_DIR}/Client.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	icestrconverterclient
	client
	"${ICE_TEST_DEPS}"
)

CreateExecutableIceTest(Ice
	"${ice_strconverter_includes}"
	${ICE_TEST_OUTPUT_DIR}/stringConverter
	${CMAKE_CURRENT_SOURCE_DIR}/Server.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	icestrconverterserver
	server
	"${ICE_TEST_DEPS}"
)
