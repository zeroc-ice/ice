set(ice_plugin_includes
	${PROJECT_SOURCE_DIR}/cpp/include
	${PROJECT_SOURCE_DIR}/cpp/src
	${PROJECT_SOURCE_DIR}/cpp/test/include
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/Ice
	${CMAKE_CURRENT_BINARY_DIR}
	${GENERATED_SLICES_DIR}
)

CreateExecutableTest(Ice
	"${ice_plugin_includes}"
	${ICE_TEST_OUTPUT_DIR}/plugin
	${CMAKE_CURRENT_SOURCE_DIR}/Client.cpp
	icepluginclient
	client
	"${ICE_TEST_DEPS}"
)

CreateLibraryTest(Ice
	"${ice_plugin_includes}"
	${ICE_TEST_OUTPUT_DIR}/plugin
	${CMAKE_CURRENT_SOURCE_DIR}/Plugin.cpp
	iceplugintestplugin
	testplugin
	"${ICE_TEST_DEPS}"
)
