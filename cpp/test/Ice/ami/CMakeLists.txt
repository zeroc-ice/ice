set(ice_ami_includes
	${PROJECT_SOURCE_DIR}/cpp/include
	${PROJECT_SOURCE_DIR}/cpp/test/include
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/Ice
	${CMAKE_CURRENT_BINARY_DIR}
	${GENERATED_SLICES_DIR}
)

set(ami_client_sources
	${CMAKE_CURRENT_SOURCE_DIR}/AllTests.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Client.cpp
)

CreateExecutableIceTest(Ice
	"${ice_ami_includes}"
	${ICE_TEST_OUTPUT_DIR}/ami 
	"${ami_client_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	iceamiclient
	client 
	"${ICE_TEST_DEPS}"
)

set(ami_collocated_sources
	${CMAKE_CURRENT_SOURCE_DIR}/AllTests.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Collocated.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.h
)

CreateExecutableIceTest(Ice
	"${ice_ami_includes}"
	${ICE_TEST_OUTPUT_DIR}/ami 
	"${ami_collocated_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	iceamicollocated
	collocated 
	"${ICE_TEST_DEPS}"
)

set(ami_server_sources
	${CMAKE_CURRENT_SOURCE_DIR}/Server.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.h
)

CreateExecutableIceTest(Ice
	"${ice_ami_includes}"
	${ICE_TEST_OUTPUT_DIR}/ami 
	"${ami_server_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	iceamiserver
	server 
	"${ICE_TEST_DEPS}"
)
