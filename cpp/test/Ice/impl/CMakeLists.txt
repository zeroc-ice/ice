set(ice_impl_includes
	${PROJECT_SOURCE_DIR}/cpp/include
	${PROJECT_SOURCE_DIR}/cpp/test/include
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/Ice
	${CMAKE_CURRENT_BINARY_DIR}
	${GENERATED_SLICES_DIR}
)

CreateExecutableImplIceTest(Ice
	"${ice_impl_includes}"
	${ICE_TEST_OUTPUT_DIR}/impl 
	${CMAKE_CURRENT_SOURCE_DIR}/Server.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	iceimplserver
	server
	"${ICE_TEST_DEPS}"
)

CreateExecutableImplIceTest(Ice
	"${ice_impl_includes}"
	${ICE_TEST_OUTPUT_DIR}/impl 
	${CMAKE_CURRENT_SOURCE_DIR}/ServerAMD.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestAMD.ice
	iceimplserveramd
	serveramd
	"${ICE_TEST_DEPS}"
)
