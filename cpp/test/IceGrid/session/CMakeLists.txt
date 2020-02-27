if(NOT ${CMAKE_CURRENT_BINARY_DIR}/IceGrid)
	file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/IceGrid)
endif()

set(GENERATED_SESSION_HEADERS ${CMAKE_CURRENT_BINARY_DIR}/IceGrid/Test.h)
set(GENERATED_SESSION_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/IceGrid/Test.cpp)

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
	set(SESSION_DEPS 
		ice
		icessl
		glacier2
		icegrid
		testcommon
	)
elseif(BUILD_ICE_CPP11)
	set(SESSION_DEPS 
		ice++11
		icessl++11
		glacier2++11
		icegrid++11
		testcommon++11
	)
endif()

add_executable(icegridsessionclient)

target_sources(icegridsessionclient
	PRIVATE
		AllTests.cpp
		Client.cpp
		${GENERATED_SESSION_HEADERS}
		${GENERATED_SESSION_SOURCES}
)

target_include_directories(icegridsessionclient
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/IceGrid
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET icegridsessionclient PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET icegridsessionclient PROPERTY CXX_STANDARD 11)
endif()


target_link_libraries(icegridsessionclient PRIVATE ${SESSION_DEPS})

target_compile_definitions(icegridsessionclient PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Test.ice
	)

	source_group("Source Files/Generated"
		FILES
			${GENERATED_SESSION_SOURCES}
	)

	source_group("Header Files/Generated"
		FILES
			${GENERATED_SESSION_HEADERS}
	)

	target_compile_options(icegridsessionclient
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	set_property(TARGET icegridsessionclient PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
endif()

install(TARGETS icegridsessionclient DESTINATION tests/bin)

add_executable(icegridsessionserver)

target_sources(icegridsessionserver
	PRIVATE
		Server.cpp
		${GENERATED_SESSION_HEADERS}
		${GENERATED_SESSION_SOURCES}
)

target_include_directories(icegridsessionserver
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/IceGrid
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET icegridsessionserver PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET icegridsessionserver PROPERTY CXX_STANDARD 11)
endif()

target_link_libraries(icegridsessionserver PRIVATE ${SESSION_DEPS})

target_compile_definitions(icegridsessionserver PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Test.ice
	)

	source_group("Source Files/Generated"
		FILES
			${GENERATED_SESSION_SOURCES}
	)

	source_group("Header Files/Generated"
		FILES
			${GENERATED_SESSION_HEADERS}
	)

	target_compile_options(icegridsessionserver
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	set_property(TARGET icegridsessionserver PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
endif()

install(TARGETS icegridsessionserver DESTINATION tests/bin)

add_executable(icegridsessionverifier)

target_sources(icegridsessionverifier
	PRIVATE
		PermissionsVerifier.cpp
		${GENERATED_SESSION_HEADERS}
		${GENERATED_SESSION_SOURCES}
)

target_include_directories(icegridsessionverifier
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/IceGrid
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET icegridsessionverifier PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET icegridsessionverifier PROPERTY CXX_STANDARD 11)
endif()

target_link_libraries(icegridsessionverifier PRIVATE ${SESSION_DEPS})

target_compile_definitions(icegridsessionverifier PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	target_compile_options(icegridsessionverifier
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	set_property(TARGET icegridsessionverifier PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
endif()

install(TARGETS icegridsessionverifier DESTINATION tests/bin)
