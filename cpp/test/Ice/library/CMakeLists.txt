set(ice_library_includes
	${PROJECT_SOURCE_DIR}/cpp/include
	${PROJECT_SOURCE_DIR}/cpp/src
	${PROJECT_SOURCE_DIR}/cpp/test/include
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/Ice
	${CMAKE_CURRENT_BINARY_DIR}
	${GENERATED_SLICES_DIR}
)

set(library_alltests_deps
	icelibraryconsumer 
	icelibrarygencode
	${ICE_TEST_DEPS}
)

CreateLibraryTest(Ice
	"${ice_library_includes}"
	${ICE_TEST_OUTPUT_DIR}/library
	${CMAKE_CURRENT_SOURCE_DIR}/Alltests.cpp
	icelibraryalltests
	alltests
	"${library_alltests_deps}"
)

set(library_client_deps
	icelibraryalltests
	${ICE_TEST_DEPS}
)

CreateExecutableTest(Ice
	"${ice_library_includes}"
	${ICE_TEST_OUTPUT_DIR}/library
	${CMAKE_CURRENT_SOURCE_DIR}/Client.cpp
	icelibraryclient
	client
	"${library_client_deps}"
)

set(library_consumer_deps
	icelibrarygencode
	${ICE_TEST_DEPS}
)

CreateLibraryTest(Ice
	"${ice_library_includes}"
	${ICE_TEST_OUTPUT_DIR}/library
	${CMAKE_CURRENT_SOURCE_DIR}/Consumer.cpp
	icelibraryconsumer
	consumer
	"${library_consumer_deps}"
)

CreateLibraryIceTest(Ice
	"${ice_library_includes}"
	${ICE_TEST_OUTPUT_DIR}/library
	""
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	icelibrarygencode
	gencode
	"${ICE_TEST_DEPS}"
)