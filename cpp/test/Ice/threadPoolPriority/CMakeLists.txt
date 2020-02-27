if(NOT ${CMAKE_CURRENT_BINARY_DIR}/Ice)
	file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Ice)
endif()

set(GENERATED_THREADPOOLPRI_HEADERS ${CMAKE_CURRENT_BINARY_DIR}/Ice/Test.h)
set(GENERATED_THREADPOOLPRI_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/Ice/Test.cpp)

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
	set(THREADPOOLPRI_DEPS 
		ice
		testcommon
	)
elseif(BUILD_ICE_CPP11)
	set(THREADPOOLPRI_DEPS 
		ice++11
		testcommon++11
	)
endif()

add_executable(icethreadpoolpriclient)

target_sources(icethreadpoolpriclient
	PRIVATE
		AllTests.cpp
		Client.cpp
		${GENERATED_THREADPOOLPRI_HEADERS}
		${GENERATED_THREADPOOLPRI_SOURCES}
)

target_include_directories(icethreadpoolpriclient
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/Ice
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET icethreadpoolpriclient PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET icethreadpoolpriclient PROPERTY CXX_STANDARD 11)
endif()

target_link_libraries(icethreadpoolpriclient PRIVATE ${THREADPOOLPRI_DEPS})

target_compile_definitions(icethreadpoolpriclient PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Test.ice
	)

	source_group("Source Files/Generated"
		FILES
			${GENERATED_THREADPOOLPRI_SOURCES}
	)

	source_group("Header Files/Generated"
		FILES
			${GENERATED_THREADPOOLPRI_HEADERS}
	)

	target_compile_options(icethreadpoolpriclient
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	set_property(TARGET icethreadpoolpriclient PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
endif()

install(TARGETS icethreadpoolpriclient DESTINATION tests/bin)

add_executable(icethreadpoolpriserver)

target_sources(icethreadpoolpriserver
	PRIVATE
		Server.cpp
		TestI.cpp
		${GENERATED_THREADPOOLPRI_HEADERS}
		${GENERATED_THREADPOOLPRI_SOURCES}
)

target_include_directories(icethreadpoolpriserver
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/Ice
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET icethreadpoolpriserver PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET icethreadpoolpriserver PROPERTY CXX_STANDARD 11)
endif()

target_link_libraries(icethreadpoolpriserver PRIVATE ${THREADPOOLPRI_DEPS})

target_compile_definitions(icethreadpoolpriserver PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Test.ice
	)

	source_group("Source Files/Generated"
		FILES
			${GENERATED_THREADPOOLPRI_SOURCES}
	)

	source_group("Header Files/Generated"
		FILES
			${GENERATED_THREADPOOLPRI_HEADERS}
	)

	target_compile_options(icethreadpoolpriserver
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	set_property(TARGET icethreadpoolpriserver PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
endif()

install(TARGETS icethreadpoolpriserver DESTINATION tests/bin)

add_executable(icethreadpoolpriservercustom)

target_sources(icethreadpoolpriservercustom
	PRIVATE
		ServerCustomThreadPool.cpp
		TestI.cpp
		${GENERATED_THREADPOOLPRI_HEADERS}
		${GENERATED_THREADPOOLPRI_SOURCES}
)

target_include_directories(icethreadpoolpriservercustom
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/Ice
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET icethreadpoolpriservercustom PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET icethreadpoolpriservercustom PROPERTY CXX_STANDARD 11)
endif()

target_link_libraries(icethreadpoolpriservercustom PRIVATE ${THREADPOOLPRI_DEPS})

target_compile_definitions(icethreadpoolpriservercustom PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Test.ice
	)

	source_group("Source Files/Generated"
		FILES
			${GENERATED_THREADPOOLPRI_SOURCES}
	)

	source_group("Header Files/Generated"
		FILES
			${GENERATED_THREADPOOLPRI_HEADERS}
	)

	target_compile_options(icethreadpoolpriservercustom
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	set_property(TARGET icethreadpoolpriservercustom PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
endif()

install(TARGETS icethreadpoolpriservercustom DESTINATION tests/bin)

