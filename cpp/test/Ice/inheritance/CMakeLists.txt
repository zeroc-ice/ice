set(ice_inheritance_includes
	${PROJECT_SOURCE_DIR}/cpp/include
	${PROJECT_SOURCE_DIR}/cpp/test/include
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/Ice
	${CMAKE_CURRENT_BINARY_DIR}
	${GENERATED_SLICES_DIR}
)

set(inheritance_client_sources
	${CMAKE_CURRENT_SOURCE_DIR}/AllTests.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Client.cpp
)

CreateExecutableIceTest(Ice
	"${ice_inheritance_includes}"
	${ICE_TEST_OUTPUT_DIR}/inheritance 
	"${inheritance_client_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	iceinheritanceclient
	client 
	"${ICE_TEST_DEPS}"
)

set(inheritance_collocated_sources
	${CMAKE_CURRENT_SOURCE_DIR}/AllTests.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Collocated.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.h
)

CreateExecutableIceTest(Ice
	"${ice_inheritance_includes}"
	${ICE_TEST_OUTPUT_DIR}/inheritance 
	"${inheritance_collocated_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	iceinheritancecollocated
	collocated 
	"${ICE_TEST_DEPS}"
)

set(inheritance_server_sources
	${CMAKE_CURRENT_SOURCE_DIR}/Server.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.h
)

CreateExecutableIceTest(Ice
	"${ice_inheritance_includes}"
	${ICE_TEST_OUTPUT_DIR}/inheritance 
	"${inheritance_server_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	iceinheritanceserver
	server 
	"${ICE_TEST_DEPS}"
)
