set(ice_thrdpoolpri_includes
	${PROJECT_SOURCE_DIR}/cpp/include
	${PROJECT_SOURCE_DIR}/cpp/test/include
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/Ice
	${CMAKE_CURRENT_BINARY_DIR}
	${GENERATED_SLICES_DIR}
)

set(threadPoolPriority_client_sources
	${CMAKE_CURRENT_SOURCE_DIR}/AllTests.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Client.cpp
)

CreateExecutableIceTest(Ice
	"${ice_thrdpoolpri_includes}"
	${ICE_TEST_OUTPUT_DIR}/threadPoolPriority 
	"${threadPoolPriority_client_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	icethrdpoolpriclient
	client 
	"${ICE_TEST_DEPS}"
)

set(threadPoolPriority_server_sources
	${CMAKE_CURRENT_SOURCE_DIR}/Server.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.h
)

CreateExecutableIceTest(Ice
	"${ice_thrdpoolpri_includes}"
	${ICE_TEST_OUTPUT_DIR}/threadPoolPriority 
	"${threadPoolPriority_server_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	icethrdpoolpriserver
	server
	"${ICE_TEST_DEPS}"
)

set(threadPoolPriority_srvcustom_sources
	${CMAKE_CURRENT_SOURCE_DIR}/ServerCustomThreadPool.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.h
)

CreateExecutableIceTest(Ice
	"${ice_thrdpoolpri_includes}"
	${ICE_TEST_OUTPUT_DIR}/threadPoolPriority 
	"${threadPoolPriority_srvcustom_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	icethrdpoolprisrvcustom
	servercustom
	"${ICE_TEST_DEPS}"
)
