if(NOT ${CMAKE_CURRENT_BINARY_DIR}/IceGrid)
	file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/IceGrid)
endif()

set(GENERATED_ACTIVATION_HEADERS ${CMAKE_CURRENT_BINARY_DIR}/IceGrid/Test.h)
set(GENERATED_ACTIVATION_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/IceGrid/Test.cpp)

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
	set(ACTIVATION_DEPS 
		ice
		glacier2
		icegrid
		testcommon
	)
elseif(BUILD_ICE_CPP11)
	set(ACTIVATION_DEPS 
		ice++11
		glacier2++11
		icegrid++11
		testcommon++11
	)
endif()

add_executable(icegridactivationclient)

target_sources(icegridactivationclient
	PRIVATE
		AllTests.cpp
		Client.cpp
		${GENERATED_ACTIVATION_HEADERS}
		${GENERATED_ACTIVATION_SOURCES}
)

target_include_directories(icegridactivationclient
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/IceGrid
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET icegridactivationclient PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET icegridactivationclient PROPERTY CXX_STANDARD 11)
endif()

target_link_libraries(icegridactivationclient PRIVATE ${ACTIVATION_DEPS})

target_compile_definitions(icegridactivationclient PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Test.ice
	)

	source_group("Source Files/Generated"
		FILES
			${GENERATED_ACTIVATION_SOURCES}
	)

	source_group("Header Files/Generated"
		FILES
			${GENERATED_ACTIVATION_HEADERS}
	)

	target_compile_options(icegridactivationclient
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	set_property(TARGET icegridactivationclient PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
endif()

install(TARGETS icegridactivationclient DESTINATION tests/bin)

add_executable(icegridactivationserver)

target_sources(icegridactivationserver
	PRIVATE
		Server.cpp
		TestI.cpp
		${GENERATED_ACTIVATION_HEADERS}
		${GENERATED_ACTIVATION_SOURCES}
)

target_include_directories(icegridactivationserver
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/IceGrid
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET icegridactivationserver PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET icegridactivationserver PROPERTY CXX_STANDARD 11)
endif()

target_link_libraries(icegridactivationserver PRIVATE ${ACTIVATION_DEPS})

target_compile_definitions(icegridactivationserver PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Test.ice
	)

	source_group("Source Files/Generated"
		FILES
			${GENERATED_ACTIVATION_SOURCES}
	)

	source_group("Header Files/Generated"
		FILES
			${GENERATED_ACTIVATION_HEADERS}
	)

	target_compile_options(icegridactivationserver
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	set_property(TARGET icegridactivationserver PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
endif()

install(TARGETS icegridactivationserver DESTINATION tests/bin)
