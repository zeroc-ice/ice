set(icebox_admin_includes
	${PROJECT_SOURCE_DIR}/cpp/include
	${PROJECT_SOURCE_DIR}/cpp/test/include
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/IceBox
	${CMAKE_CURRENT_BINARY_DIR}
	${GENERATED_SLICES_DIR}
)

set(admin_client_sources
	${CMAKE_CURRENT_SOURCE_DIR}/AllTests.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Client.cpp
)

CreateExecutableIceTest(IceBox
	"${icebox_admin_includes}"
	${ICEBOX_TEST_OUTPUT_DIR}/admin
	"${admin_client_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	iceboxadminclient
	client
	"${ICEBOX_TEST_DEPS}"
)

set(admin_testsvc_sources
	${CMAKE_CURRENT_SOURCE_DIR}/Service.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.h
)

CreateLibraryIceTest(IceBox
	"${icebox_admin_includes}"
	${ICEBOX_TEST_OUTPUT_DIR}/admin
	"${admin_testsvc_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	iceboxadmintestsvc
	testservice
	"${ICEBOX_TEST_SERVICE_DEPS}"
)
