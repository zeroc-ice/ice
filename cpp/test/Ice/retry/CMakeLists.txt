set(ice_retry_includes
	${PROJECT_SOURCE_DIR}/cpp/include
	${PROJECT_SOURCE_DIR}/cpp/test/include
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/Ice
	${CMAKE_CURRENT_BINARY_DIR}
	${GENERATED_SLICES_DIR}
)

set(retry_client_sources
	${CMAKE_CURRENT_SOURCE_DIR}/AllTests.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Client.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/InstrumentationI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/InstrumentationI.h
)

CreateExecutableIceTest(Ice
	"${ice_retry_includes}"
	${ICE_TEST_OUTPUT_DIR}/retry 
	"${retry_client_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	iceretryclient
	client 
	"${ICE_TEST_DEPS}"
)

set(retry_collocated_sources
	${CMAKE_CURRENT_SOURCE_DIR}/AllTests.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Collocated.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/InstrumentationI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/InstrumentationI.h
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.h
)

CreateExecutableIceTest(Ice
	"${ice_retry_includes}"
	${ICE_TEST_OUTPUT_DIR}/retry 
	"${retry_collocated_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	iceretrycollocated
	collocated 
	"${ICE_TEST_DEPS}"
)

set(retry_server_sources
	${CMAKE_CURRENT_SOURCE_DIR}/InstrumentationI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/InstrumentationI.h
	${CMAKE_CURRENT_SOURCE_DIR}/Server.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.h
)

CreateExecutableIceTest(Ice
	"${ice_retry_includes}"
	${ICE_TEST_OUTPUT_DIR}/retry 
	"${retry_server_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	iceretryserver
	server 
	"${ICE_TEST_DEPS}"
)
