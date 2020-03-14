set(ice_facets_includes
	${PROJECT_SOURCE_DIR}/cpp/include
	${PROJECT_SOURCE_DIR}/cpp/test/include
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/Ice
	${CMAKE_CURRENT_BINARY_DIR}
	${GENERATED_SLICES_DIR}
)

set(facets_client_sources
	${CMAKE_CURRENT_SOURCE_DIR}/AllTests.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Client.cpp
)

CreateExecutableIceTest(Ice
	"${ice_facets_includes}"
	${ICE_TEST_OUTPUT_DIR}/facets 
	"${facets_client_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	icefacetsclient
	client 
	"${ICE_TEST_DEPS}"
)

set(facets_collocated_sources
	${CMAKE_CURRENT_SOURCE_DIR}/AllTests.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Collocated.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.h
)

CreateExecutableIceTest(Ice
	"${ice_facets_includes}"
	${ICE_TEST_OUTPUT_DIR}/facets 
	"${facets_collocated_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	icefacetscollocated
	collocated 
	"${ICE_TEST_DEPS}"
)

set(facets_server_sources
	${CMAKE_CURRENT_SOURCE_DIR}/Server.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.h
)

CreateExecutableIceTest(Ice
	"${ice_facets_includes}"
	${ICE_TEST_OUTPUT_DIR}/facets 
	"${facets_server_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	icefacetsserver
	server 
	"${ICE_TEST_DEPS}"
)