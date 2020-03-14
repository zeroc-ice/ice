set(glacier2_staticfilter_includes
	${PROJECT_SOURCE_DIR}/cpp/include
	${PROJECT_SOURCE_DIR}/cpp/test/include
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/Glacier2
	${CMAKE_CURRENT_BINARY_DIR}
	${GENERATED_SLICES_DIR}
)

set(staticFiltering_client_sources
	${CMAKE_CURRENT_SOURCE_DIR}/BackendI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/BackendI.h
	${CMAKE_CURRENT_SOURCE_DIR}/Client.cpp
)

CreateExecutableIceTest(Glacier2
	"${glacier2_staticfilter_includes}"
	${GLACIER2_TEST_OUTPUT_DIR}/staticFiltering
	"${staticFiltering_client_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Backend.ice
	glacier2staticfilterclient
	client
	"${GLACIER2_TEST_DEPS}"
)

set(staticFiltering_server_sources
	${CMAKE_CURRENT_SOURCE_DIR}/BackendI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/BackendI.h
	${CMAKE_CURRENT_SOURCE_DIR}/Server.cpp
)

CreateExecutableIceTest(Glacier2
	"${glacier2_staticfilter_includes}"
	${GLACIER2_TEST_OUTPUT_DIR}/staticFiltering
	"${staticFiltering_server_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Backend.ice
	glacier2staticfilterserver
	server
	"${GLACIER2_TEST_DEPS}"
)
