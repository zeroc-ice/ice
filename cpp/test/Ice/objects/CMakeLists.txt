set(ice_objects_includes
	${PROJECT_SOURCE_DIR}/cpp/include
	${PROJECT_SOURCE_DIR}/cpp/test/include
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/Ice
	${CMAKE_CURRENT_BINARY_DIR}
	${GENERATED_SLICES_DIR}
)

set(objects_slices
	${CMAKE_CURRENT_SOURCE_DIR}/Derived.ice
	${CMAKE_CURRENT_SOURCE_DIR}/DerivedEx.ice
	${CMAKE_CURRENT_SOURCE_DIR}/Forward.ice
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
)

set(objects_client_sources
	${CMAKE_CURRENT_SOURCE_DIR}/AllTests.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Client.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.h
)

CreateExecutableInternalIceTest(Ice
	"${ice_objects_includes}"
	${ICE_TEST_OUTPUT_DIR}/objects 
	"${objects_client_sources}"
	"${objects_slices}"
	iceobjectsclient
	client 
	"${ICE_TEST_DEPS}"
)

set(objects_collcated_sources
	${CMAKE_CURRENT_SOURCE_DIR}/AllTests.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Collocated.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.h
	${CMAKE_CURRENT_SOURCE_DIR}/TestIntfI.cpp
)

CreateExecutableInternalIceTest(Ice
	"${ice_objects_includes}"
	${ICE_TEST_OUTPUT_DIR}/objects 
	"${objects_collcated_sources}"
	"${objects_slices}"
	iceobjectscollocated
	collocated 
	"${ICE_TEST_DEPS}"
)

set(objects_server_sources
	${CMAKE_CURRENT_SOURCE_DIR}/Server.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.h
	${CMAKE_CURRENT_SOURCE_DIR}/TestIntfI.cpp
)

CreateExecutableInternalIceTest(Ice
	"${ice_objects_includes}"
	${ICE_TEST_OUTPUT_DIR}/objects 
	"${objects_server_sources}"
	"${objects_slices}"
	iceobjectsserver
	server
	"${ICE_TEST_DEPS}"
)
