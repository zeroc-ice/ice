if(NOT ${CMAKE_CURRENT_BINARY_DIR}/IceGrid)
	file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/IceGrid)
endif()

set(GENERATED_UPDATE_HEADERS ${CMAKE_CURRENT_BINARY_DIR}/IceGrid/Test.h)
set(GENERATED_UPDATE_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/IceGrid/Test.cpp)

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

add_executable(icegridnorestartupdateclient)

target_sources(icegridnorestartupdateclient
	PRIVATE
		AllTests.cpp
		Client.cpp
		${GENERATED_UPDATE_HEADERS}
		${GENERATED_UPDATE_SOURCES}
)

target_include_directories(icegridnorestartupdateclient
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/IceGrid
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET icegridnorestartupdateclient PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET icegridnorestartupdateclient PROPERTY CXX_STANDARD 11)
endif()

target_link_libraries(icegridnorestartupdateclient PRIVATE ${CLIENT_DEPS})

target_compile_definitions(icegridnorestartupdateclient PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Test.ice
	)

	source_group("Source Files/Generated"
		FILES
			${GENERATED_UPDATE_SOURCES}
	)

	source_group("Header Files/Generated"
		FILES
			${GENERATED_UPDATE_HEADERS}
	)

	target_compile_options(icegridnorestartupdateclient
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	set_property(TARGET icegridnorestartupdateclient PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
endif()

install(TARGETS icegridnorestartupdateclient DESTINATION tests/bin)

add_executable(icegridnorestartupdateserver)

target_sources(icegridnorestartupdateserver
	PRIVATE
		Server.cpp
		TestI.cpp
		${GENERATED_UPDATE_HEADERS}
		${GENERATED_UPDATE_SOURCES}
)

target_include_directories(icegridnorestartupdateserver
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/IceGrid
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET icegridnorestartupdateserver PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET icegridnorestartupdateserver PROPERTY CXX_STANDARD 11)
endif()

target_link_libraries(icegridnorestartupdateserver PRIVATE ${CLIENT_DEPS})

target_compile_definitions(icegridnorestartupdateserver PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Test.ice
	)

	source_group("Source Files/Generated"
		FILES
			${GENERATED_UPDATE_SOURCES}
	)

	source_group("Header Files/Generated"
		FILES
			${GENERATED_UPDATE_HEADERS}
	)

	target_compile_options(icegridnorestartupdateserver
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	set_property(TARGET icegridnorestartupdateserver PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
endif()

install(TARGETS icegridnorestartupdateserver DESTINATION tests/bin)

add_library(icegridnorestartupdatetestservice)

target_sources(icegridnorestartupdatetestservice
	PRIVATE
		Service.cpp
		TestI.cpp
		${GENERATED_UPDATE_HEADERS}
		${GENERATED_UPDATE_SOURCES}
)

target_include_directories(icegridnorestartupdatetestservice
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/IceGrid
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_target_properties(icegridnorestartupdatetestservice 
		PROPERTIES 
			CXX_STANDARD 98
			OUTPUT_NAME testservice
	)
elseif(BUILD_ICE_CPP11)
	set_target_properties(icegridnorestartupdatetestservice 
		PROPERTY 
			CXX_STANDARD 11
			OUTPUT_NAME testservice++11
	)
endif()

target_link_libraries(icegridnorestartupdatetestservice PRIVATE ${SERVICE_DEPS})

target_compile_definitions(icegridnorestartupdatetestservice PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Test.ice
	)

	source_group("Source Files/Generated"
		FILES
			${GENERATED_UPDATE_SOURCES}
	)

	source_group("Header Files/Generated"
		FILES
			${GENERATED_UPDATE_HEADERS}
	)

	target_compile_options(icegridnorestartupdatetestservice
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	set_property(TARGET icegridnorestartupdatetestservice PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
endif()

if(BUILD_SHARED_LIBS)
	install(TARGETS icegridnorestartupdatetestservice DESTINATION tests/bin)
else()
	install(TARGETS icegridnorestartupdatetestservice DESTINATION tests/lib)
endif()

