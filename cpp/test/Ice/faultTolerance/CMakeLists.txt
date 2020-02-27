if(NOT ${CMAKE_CURRENT_BINARY_DIR}/Ice)
	file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Ice)
endif()

set(GENERATED_FLTTOLERANCE_HEADERS ${CMAKE_CURRENT_BINARY_DIR}/Ice/Test.h)
set(GENERATED_FLTTOLERANCE_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/Ice/Test.cpp)

list(APPEND ICE_SLICE_DIRS "-I${SLICE_DIR}")
list(APPEND ICE_SLICE_PARAMS ${COMPILE_SLICES_CPP_PARAMETERS} "Ice")

set(output_files 
	${CMAKE_CURRENT_BINARY_DIR}/Ice/Test.cpp 
	${CMAKE_CURRENT_BINARY_DIR}/Ice/Test.h
)	
#function(CompileSlice slice2bin_params slice_include_paths ice_file output_dir output_file)
CompileSlice("${ICE_SLICE_PARAMS}"
	"${ICE_SLICE_DIRS}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	${CMAKE_CURRENT_BINARY_DIR}/Ice
	"${output_files}"
)

if(BUILD_ICE_CPP98)
	set(FLTTOLERANCE_DEPS 
		ice
		testcommon
	)
elseif(BUILD_ICE_CPP11)
	set(FLTTOLERANCE_DEPS 
		ice++11
		testcommon++11
	)
endif()

add_executable(iceflttoleranceclient)

target_sources(iceflttoleranceclient
	PRIVATE
		AllTests.cpp
		Client.cpp
		${GENERATED_FLTTOLERANCE_HEADERS}
		${GENERATED_FLTTOLERANCE_SOURCES}
)

target_include_directories(iceflttoleranceclient
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/Ice
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET iceflttoleranceclient PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET iceflttoleranceclient PROPERTY CXX_STANDARD 11)
endif()

target_link_libraries(iceflttoleranceclient PRIVATE ${FLTTOLERANCE_DEPS})

target_compile_definitions(iceflttoleranceclient PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Test.ice
	)

	source_group("Source Files/Generated"
		FILES
			${GENERATED_FLTTOLERANCE_SOURCES}
	)

	source_group("Header Files/Generated"
		FILES
			${GENERATED_FLTTOLERANCE_HEADERS}
	)

	target_compile_options(iceflttoleranceclient
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	set_property(TARGET iceflttoleranceclient PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
endif()

install(TARGETS iceflttoleranceclient DESTINATION tests/bin)

add_executable(iceflttoleranceserver)

target_sources(iceflttoleranceserver
	PRIVATE
		Server.cpp
		TestI.cpp
		${GENERATED_FLTTOLERANCE_HEADERS}
		${GENERATED_FLTTOLERANCE_SOURCES}
)

target_include_directories(iceflttoleranceserver
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/Ice
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET iceflttoleranceserver PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET iceflttoleranceserver PROPERTY CXX_STANDARD 11)
endif()

target_link_libraries(iceflttoleranceserver PRIVATE ${FLTTOLERANCE_DEPS})

target_compile_definitions(iceflttoleranceserver PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Test.ice
	)

	source_group("Source Files/Generated"
		FILES
			${GENERATED_FLTTOLERANCE_SOURCES}
	)

	source_group("Header Files/Generated"
		FILES
			${GENERATED_FLTTOLERANCE_HEADERS}
	)

	target_compile_options(iceflttoleranceserver
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	set_property(TARGET iceflttoleranceserver PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
endif()

install(TARGETS iceflttoleranceserver DESTINATION tests/bin)
