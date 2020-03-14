set(slicing_obj_includes
	${PROJECT_SOURCE_DIR}/cpp/include
	${PROJECT_SOURCE_DIR}/cpp/test/include
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/Ice
	${CMAKE_CURRENT_BINARY_DIR}
	${GENERATED_SLICES_DIR}
)

set(objects_client_slices
	${CMAKE_CURRENT_SOURCE_DIR}/ClientPrivate.ice
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
)

set(objects_client_sources
	${CMAKE_CURRENT_SOURCE_DIR}/AllTests.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Client.cpp
)

CreateExecutableInternalIceTest(Ice
	"${slicing_obj_includes}"
	${ICE_TEST_OUTPUT_DIR}/slicing/objects 
	"${objects_client_sources}"
	"${objects_client_slices}"
	iceslicingobjclient
	client 
	"${ICE_TEST_DEPS}"
)

set(objects_server_slices
	${CMAKE_CURRENT_SOURCE_DIR}/ServerPrivate.ice
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
)

set(objects_server_sources
	${CMAKE_CURRENT_SOURCE_DIR}/Server.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.h
)

CreateExecutableInternalIceTest(Ice
	"${slicing_obj_includes}"
	${ICE_TEST_OUTPUT_DIR}/slicing/objects 
	"${objects_server_sources}"
	"${objects_server_slices}"
	iceslicingobjserver
	server
	"${ICE_TEST_DEPS}"
)

set(objects_serveramd_slices
	${CMAKE_CURRENT_SOURCE_DIR}/ServerPrivateAMD.ice
	${CMAKE_CURRENT_SOURCE_DIR}/TestAMD.ice
)

set(objects_serveramd_sources
	${CMAKE_CURRENT_SOURCE_DIR}/ServerAMD.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestAMDI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestAMDI.h
)

CreateExecutableInternalIceTest(Ice
	"${slicing_obj_includes}"
	${ICE_TEST_OUTPUT_DIR}/slicing/objects 
	"${objects_serveramd_sources}"
	"${objects_serveramd_slices}"
	iceslicingobjserveramd
	serveramd
	"${ICE_TEST_DEPS}"
)