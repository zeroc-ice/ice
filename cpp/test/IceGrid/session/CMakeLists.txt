set(icegrid_session_includes
	${PROJECT_SOURCE_DIR}/cpp/include
	${PROJECT_SOURCE_DIR}/cpp/test/include
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/IceGrid
	${CMAKE_CURRENT_BINARY_DIR}
	${GENERATED_SLICES_DIR}
)

set(session_client_sources
	${CMAKE_CURRENT_SOURCE_DIR}/AllTests.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Client.cpp
)

CreateExecutableIceTest(IceGrid
	"${icegrid_session_includes}"
	${ICEGRID_TEST_OUTPUT_DIR}/session
	"${session_client_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	icegridsessionclient
	client
	"${ICEGRID_TEST_DEPS}"
)

if(BUILD_ICE_CPP11)
	set(session_server_deps
		icessl++11
		${ICEGRID_TEST_DEPS}
	)
else()
	set(session_server_deps
		icessl
		${ICEGRID_TEST_DEPS}
	)
endif()

CreateExecutableIceTest(IceGrid
	"${icegrid_session_includes}"
	${ICEGRID_TEST_OUTPUT_DIR}/session
	${CMAKE_CURRENT_SOURCE_DIR}/Server.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	icegridsessionserver
	server
	"${session_server_deps}"
)

if(BUILD_ICE_CPP11)
	set(session_verifier_deps
		icessl++11
		${ICEGRID_TEST_DEPS}
	)
else()
	set(session_verifier_deps
		icessl
		${ICEGRID_TEST_DEPS}
	)
endif()

CreateExecutableIceTest(IceGrid
	"${icegrid_session_includes}"
	${ICEGRID_TEST_OUTPUT_DIR}/session
	${CMAKE_CURRENT_SOURCE_DIR}/PermissionsVerifier.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	icegridsessionpermverifier
	verifier
	"${session_verifier_deps}"
)
