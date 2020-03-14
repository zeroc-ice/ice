set(icegrid_replgrp_includes
	${PROJECT_SOURCE_DIR}/cpp/include
	${PROJECT_SOURCE_DIR}/cpp/test/include
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/IceGrid
	${CMAKE_CURRENT_BINARY_DIR}
	${GENERATED_SLICES_DIR}
)

set(replicaGroup_client_sources
	${CMAKE_CURRENT_SOURCE_DIR}/AllTests.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Client.cpp
)

CreateExecutableIceTest(IceGrid
	"${icegrid_replgrp_includes}"
	${ICEGRID_TEST_OUTPUT_DIR}/replicaGroup
	"${replicaGroup_client_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	icegridreplgrpclient
	client
	"${ICEGRID_TEST_DEPS}"
)

if(BUILD_ICE_CPP11)
	set(registryplugin_service
		glacier2++11
		icegrid++11
		iceboxlib++11
		${ICEGRID_TEST_SERVICE_DEPS}
	)
else()
	set(registryplugin_service
		glacier2
		icegrid
		iceboxlib
		${ICEGRID_TEST_SERVICE_DEPS}
	)
endif()

CreateLibraryTest(IceGrid
	"${icegrid_replgrp_includes}"
	${ICEGRID_TEST_OUTPUT_DIR}/replicaGroup
	${CMAKE_CURRENT_SOURCE_DIR}/RegistryPlugin.cpp
	icereplicagrpregistryplugin
	registryplugin
	"${registryplugin_service}"
)

set(replicaGroup_server_sources
	${CMAKE_CURRENT_SOURCE_DIR}/Server.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.h
)

CreateExecutableIceTest(IceGrid
	"${icegrid_replgrp_includes}"
	${ICEGRID_TEST_OUTPUT_DIR}/replicaGroup
	"${replicaGroup_server_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	icegridreplgrpserver
	server
	"${ICEGRID_TEST_DEPS}"
)

set(replicaGroup_service_sources
	${CMAKE_CURRENT_SOURCE_DIR}/Service.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/TestI.h
)

CreateLibraryIceTest(IceGrid
	"${icegrid_replgrp_includes}"
	${ICEGRID_TEST_OUTPUT_DIR}/replicaGroup
	"${replicaGroup_service_sources}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	icegridreplgrptestservice
	testservice
	"${ICEGRID_TEST_SERVICE_DEPS}"
)
