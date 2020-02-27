if(NOT ${CMAKE_CURRENT_BINARY_DIR}/IceGrid)
	file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/IceGrid)
endif()

set(GENERATED_REPLICA_HEADERS ${CMAKE_CURRENT_BINARY_DIR}/IceGrid/Test.h)
set(GENERATED_REPLICA_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/IceGrid/Test.cpp)

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
		glacier2
		iceboxlib
		icegrid
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
		glacier2++11
		iceboxlib++11
		icegrid++11
		testcommon++11
	)
endif()

add_executable(icegridreplicagrpclient)

target_sources(icegridreplicagrpclient
	PRIVATE
		AllTests.cpp
		Client.cpp
		${GENERATED_REPLICA_HEADERS}
		${GENERATED_REPLICA_SOURCES}
)

target_include_directories(icegridreplicagrpclient
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/IceGrid
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET icegridreplicagrpclient PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET icegridreplicagrpclient PROPERTY CXX_STANDARD 11)
endif()

target_link_libraries(icegridreplicagrpclient PRIVATE ${CLIENT_DEPS})

target_compile_definitions(icegridreplicagrpclient PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Test.ice
	)

	source_group("Source Files/Generated"
		FILES
			${GENERATED_REPLICA_SOURCES}
	)

	source_group("Header Files/Generated"
		FILES
			${GENERATED_REPLICA_HEADERS}
	)

	target_compile_options(icegridreplicagrpclient
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	set_property(TARGET icegridreplicagrpclient PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
endif()

install(TARGETS icegridreplicagrpclient DESTINATION tests/bin)

add_library(icegridreplicagrpregplugin)

target_sources(icegridreplicagrpregplugin
	PRIVATE
		RegistryPlugin.cpp
)

target_include_directories(icegridreplicagrpregplugin
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/IceGrid
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_target_properties(icegridreplicagrpregplugin 
		PROPERTIES 
			CXX_STANDARD 98
			OUTPUT_NAME registryplugin
	)
elseif(BUILD_ICE_CPP11)
	set_target_properties(icegridreplicagrpregplugin 
		PROPERTY 
			CXX_STANDARD 11
			OUTPUT_NAME registryplugin++11
	)
endif()

target_link_libraries(icegridreplicagrpregplugin PRIVATE ${SERVICE_DEPS})

target_compile_definitions(icegridreplicagrpregplugin PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	target_compile_options(icegridreplicagrpregplugin
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	set_property(TARGET icegridreplicagrpregplugin PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
endif()

if(BUILD_SHARED_LIBS)
	install(TARGETS icegridreplicagrpregplugin DESTINATION tests/bin)
else()
	install(TARGETS icegridreplicagrpregplugin DESTINATION tests/lib)
endif()

add_executable(icegridreplicagrpserver)

target_sources(icegridreplicagrpserver
	PRIVATE
		Server.cpp
		TestI.cpp
		${GENERATED_REPLICA_HEADERS}
		${GENERATED_REPLICA_SOURCES}
)

target_include_directories(icegridreplicagrpserver
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/IceGrid
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET icegridreplicagrpserver PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET icegridreplicagrpserver PROPERTY CXX_STANDARD 11)
endif()

target_link_libraries(icegridreplicagrpserver PRIVATE ${CLIENT_DEPS})

target_compile_definitions(icegridreplicagrpserver PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Test.ice
	)

	source_group("Source Files/Generated"
		FILES
			${GENERATED_REPLICA_SOURCES}
	)

	source_group("Header Files/Generated"
		FILES
			${GENERATED_REPLICA_HEADERS}
	)

	target_compile_options(icegridreplicagrpserver
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	set_property(TARGET icegridreplicagrpserver PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
endif()

install(TARGETS icegridreplicagrpserver DESTINATION tests/bin)

add_library(icegridreplicagrptestservice)

target_sources(icegridreplicagrptestservice
	PRIVATE
		Service.cpp
		TestI.cpp
		${GENERATED_REPLICA_HEADERS}
		${GENERATED_REPLICA_SOURCES}
)

target_include_directories(icegridreplicagrptestservice
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/IceGrid
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_target_properties(icegridreplicagrptestservice 
		PROPERTIES 
			CXX_STANDARD 98
			OUTPUT_NAME testservice
	)
elseif(BUILD_ICE_CPP11)
	set_target_properties(icegridreplicagrptestservice 
		PROPERTY 
			CXX_STANDARD 11
			OUTPUT_NAME testservice++11
	)
endif()

target_link_libraries(icegridreplicagrptestservice PRIVATE ${SERVICE_DEPS})

target_compile_definitions(icegridreplicagrptestservice PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Test.ice
	)

	source_group("Source Files/Generated"
		FILES
			${GENERATED_REPLICA_SOURCES}
	)

	source_group("Header Files/Generated"
		FILES
			${GENERATED_REPLICA_HEADERS}
	)

	target_compile_options(icegridreplicagrptestservice
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	set_property(TARGET icegridreplicagrptestservice PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
endif()

if(BUILD_SHARED_LIBS)
	install(TARGETS icegridreplicagrptestservice DESTINATION tests/bin)
else()
	install(TARGETS icegridreplicagrptestservice DESTINATION tests/lib)
endif()

