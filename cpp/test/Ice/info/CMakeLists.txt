set(ice_info_includes
	${PROJECT_SOURCE_DIR}/cpp/include
	${PROJECT_SOURCE_DIR}/cpp/test/include
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/Ice
	${CMAKE_CURRENT_BINARY_DIR}
	${GENERATED_SLICES_DIR}
)

set(info_client_sources
	${CMAKE_CURRENT_SOURCE_DIR}/AllTests.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Client.cpp
)

if(BUILD_ICE_CPP11)
	set(info_test_deps
		icessl++11
		${ICE_TEST_DEPS}
	)
else()
	set(info_test_deps
		icessl
		${ICE_TEST_DEPS}
	)
endif()

CreateExecutableIceTest(Ice
	"${ice_info_includes}"
	${ICE_TEST_OUTPUT_DIR}/info 
	"${info_client_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	iceinfoclient
	client 
	"${info_test_deps}"
)

set(info_server_sources
	${CMAKE_CURRENT_SOURCE_DIR}/Server.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.h
)

CreateExecutableIceTest(Ice
	"${ice_info_includes}"
	${ICE_TEST_OUTPUT_DIR}/info 
	"${info_server_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	iceinfoserver
	server
	"${info_test_deps}"
)
