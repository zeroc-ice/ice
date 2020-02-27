if(NOT ${CMAKE_CURRENT_BINARY_DIR}/IceGrid)
	file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/IceGrid)
endif()

set(GENERATED_DEPLOYER_HEADERS ${CMAKE_CURRENT_BINARY_DIR}/IceGrid/Test.h)
set(GENERATED_DEPLOYER_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/IceGrid/Test.cpp)

list(APPEND ICE_SLICE_DIRS "-I${SLICE_DIR}")
list(APPEND ICE_SLICE_PARAMS ${COMPILE_SLICES_CPP_PARAMETERS} "IceGrid")

set(output_files 
	${CMAKE_CURRENT_BINARY_DIR}/IceGrid/Test.cpp 
	${CMAKE_CURRENT_BINARY_DIR}/IceGrid/Test.h
)	
#function(CompileSlice slice2bin_params slice_include_paths ice_file output_dir output_file)
CompileSlice("${ICE_SLICE_PARAMS}"
	"${ICE_SLICE_DIRS}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	${CMAKE_CURRENT_BINARY_DIR}/IceGrid
	"${output_files}"
)

if(BUILD_ICE_CPP98)
	set(CLIENT_DEPS 
		ice
		glacier2
		icegrid
		testcommon
	)

	set(SERVICE_DEPS
		ice
		iceboxlib
		testcommon
	)
elseif(BUILD_ICE_CPP11)
	set(CLIENT_DEPS 
		ice++11
		glacier2++11
		icegrid++11
		testcommon++11
	)

	set(SERVICE_DEPS
		ice++11
		iceboxlib++11
		testcommon++11
	)
endif()

add_executable(icegriddeployerclient)

target_sources(icegriddeployerclient
	PRIVATE
		AllTests.cpp
		Client.cpp
		${GENERATED_DEPLOYER_HEADERS}
		${GENERATED_DEPLOYER_SOURCES}
)

target_include_directories(icegriddeployerclient
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/IceGrid
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET icegriddeployerclient PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET icegriddeployerclient PROPERTY CXX_STANDARD 11)
endif()

target_link_libraries(icegriddeployerclient PRIVATE ${CLIENT_DEPS})

target_compile_definitions(icegriddeployerclient PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Test.ice
	)

	source_group("Source Files/Generated"
		FILES
			${GENERATED_DEPLOYER_SOURCES}
	)

	source_group("Header Files/Generated"
		FILES
			${GENERATED_DEPLOYER_HEADERS}
	)

	target_compile_options(icegriddeployerclient
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	set_property(TARGET icegriddeployerclient PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
endif()

install(TARGETS icegriddeployerclient DESTINATION tests/bin)

add_executable(icegriddeployerserver)

target_sources(icegriddeployerserver
	PRIVATE
		Server.cpp
		TestI.cpp
		${GENERATED_DEPLOYER_HEADERS}
		${GENERATED_DEPLOYER_SOURCES}
)

target_include_directories(icegriddeployerserver
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/IceGrid
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET icegriddeployerserver PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET icegriddeployerserver PROPERTY CXX_STANDARD 11)
endif()

target_link_libraries(icegriddeployerserver PRIVATE ${CLIENT_DEPS})

target_compile_definitions(icegriddeployerserver PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Test.ice
	)

	source_group("Source Files/Generated"
		FILES
			${GENERATED_DEPLOYER_SOURCES}
	)

	source_group("Header Files/Generated"
		FILES
			${GENERATED_DEPLOYER_HEADERS}
	)

	target_compile_options(icegriddeployerserver
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	set_property(TARGET icegriddeployerserver PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
endif()

install(TARGETS icegriddeployerserver DESTINATION tests/bin)

add_library(icegriddeployertestservice)

target_sources(icegriddeployertestservice
	PRIVATE
		Service.cpp
		TestI.cpp
		${GENERATED_DEPLOYER_HEADERS}
		${GENERATED_DEPLOYER_SOURCES}
)

target_include_directories(icegriddeployertestservice
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/IceGrid
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_target_properties(icegriddeployertestservice 
		PROPERTIES 
			CXX_STANDARD 98
			OUTPUT_NAME testservice
	)
elseif(BUILD_ICE_CPP11)
	set_target_properties(icegriddeployertestservice 
		PROPERTY 
			CXX_STANDARD 11
			OUTPUT_NAME testservice++11
	)
endif()

target_link_libraries(icegriddeployertestservice PRIVATE ${SERVICE_DEPS})

target_compile_definitions(icegriddeployertestservice PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Test.ice
	)

	source_group("Source Files/Generated"
		FILES
			${GENERATED_DEPLOYER_SOURCES}
	)

	source_group("Header Files/Generated"
		FILES
			${GENERATED_DEPLOYER_HEADERS}
	)

	target_compile_options(icegriddeployertestservice
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	set_property(TARGET icegriddeployertestservice PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
endif()

if(BUILD_SHARED_LIBS)
	install(TARGETS icegriddeployertestservice DESTINATION tests/bin)
else()
	install(TARGETS icegriddeployertestservice DESTINATION tests/lib)
endif()

