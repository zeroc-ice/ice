if(NOT ${CMAKE_CURRENT_BINARY_DIR}/IceGrid)
	file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/IceGrid)
endif()

set(GENERATED_ALLOCATION_HEADERS ${CMAKE_CURRENT_BINARY_DIR}/IceGrid/Test.h)
set(GENERATED_ALLOCATION_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/IceGrid/Test.cpp)

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
	set(ALLOCATION_DEPS 
		ice
		glacier2
		icegrid
		testcommon
	)
elseif(BUILD_ICE_CPP11)
	set(ALLOCATION_DEPS 
		ice++11
		glacier2++11
		icegrid++11
		testcommon++11
	)
endif()

add_executable(icegridallocationclient)

target_sources(icegridallocationclient
	PRIVATE
		AllTests.cpp
		Client.cpp
		${GENERATED_ALLOCATION_HEADERS}
		${GENERATED_ALLOCATION_SOURCES}
)

target_include_directories(icegridallocationclient
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/IceGrid
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET icegridallocationclient PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET icegridallocationclient PROPERTY CXX_STANDARD 11)
endif()


target_link_libraries(icegridallocationclient PRIVATE ${ALLOCATION_DEPS})

target_compile_definitions(icegridallocationclient PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Test.ice
	)

	source_group("Source Files/Generated"
		FILES
			${GENERATED_ALLOCATION_SOURCES}
	)

	source_group("Header Files/Generated"
		FILES
			${GENERATED_ALLOCATION_HEADERS}
	)

	target_compile_options(icegridallocationclient
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	set_property(TARGET icegridallocationclient PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
endif()

install(TARGETS icegridallocationclient DESTINATION tests/bin)

add_executable(icegridallocationserver)

target_sources(icegridallocationserver
	PRIVATE
		Server.cpp
		TestI.cpp
		${GENERATED_ALLOCATION_HEADERS}
		${GENERATED_ALLOCATION_SOURCES}
)

target_include_directories(icegridallocationserver
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/IceGrid
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET icegridallocationserver PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET icegridallocationserver PROPERTY CXX_STANDARD 11)
endif()

target_link_libraries(icegridallocationserver PRIVATE ${ALLOCATION_DEPS})

target_compile_definitions(icegridallocationserver PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Test.ice
	)

	source_group("Source Files/Generated"
		FILES
			${GENERATED_ALLOCATION_SOURCES}
	)

	source_group("Header Files/Generated"
		FILES
			${GENERATED_ALLOCATION_HEADERS}
	)

	target_compile_options(icegridallocationserver
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	set_property(TARGET icegridallocationserver PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
endif()

install(TARGETS icegridallocationserver DESTINATION tests/bin)

add_executable(icegridallocationverifier)

target_sources(icegridallocationverifier
	PRIVATE
		PermissionsVerifier.cpp
		${GENERATED_ALLOCATION_HEADERS}
		${GENERATED_ALLOCATION_SOURCES}
)

target_include_directories(icegridallocationverifier
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/IceGrid
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET icegridallocationverifier PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET icegridallocationverifier PROPERTY CXX_STANDARD 11)
endif()

target_link_libraries(icegridallocationverifier PRIVATE ${ALLOCATION_DEPS})

target_compile_definitions(icegridallocationverifier PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	target_compile_options(icegridallocationverifier
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	set_property(TARGET icegridallocationverifier PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
endif()

install(TARGETS icegridallocationverifier DESTINATION tests/bin)
