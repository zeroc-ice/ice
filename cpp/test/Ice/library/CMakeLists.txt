if(NOT ${CMAKE_CURRENT_BINARY_DIR}/Ice)
	file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Ice)
endif()

set(GENERATED_LIBRARY_HEADERS ${CMAKE_CURRENT_BINARY_DIR}/Ice/Test.h)
set(GENERATED_LIBRARY_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/Ice/Test.cpp)

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
	set(LIBRARY_DEPS 
		ice
		testcommon
	)
elseif(BUILD_ICE_CPP11)
	set(LIBRARY_DEPS 
		ice++11
		testcommon++11
	)
endif()

if(BUILD_ICE_CPP98)
	add_library(icelibrarygencode)
	set_property(TARGET icelibrarygencode
		PROPERTY
			OUTPUT_NAME gencode${zeroc-ice_VERSION_MAJOR}${zeroc-ice_VERSION_MINOR}
	)
elseif(BUILD_ICE_CPP11)
	add_library(icelibrarygencode++11)
	set_property(TARGET icelibrarygencode++11
		PROPERTY
			OUTPUT_NAME gencode${zeroc-ice_VERSION_MAJOR}${zeroc-ice_VERSION_MINOR}++11
	)
endif()

target_sources(icelibrarygencode
	PRIVATE
		${GENERATED_LIBRARY_HEADERS}
		${GENERATED_LIBRARY_SOURCES}
)

target_include_directories(icelibrarygencode
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/src
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/Ice
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET icelibrarygencode PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET icelibrarygencode PROPERTY CXX_STANDARD 11)
endif()

target_link_libraries(icelibrarygencode PRIVATE ${LIBRARY_DEPS})

target_compile_definitions(icelibrarygencode PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Test.ice
	)

	source_group("Source Files/Generated"
		FILES
			${GENERATED_LIBRARY_SOURCES}
	)

	source_group("Header Files/Generated"
		FILES
			${GENERATED_LIBRARY_HEADERS}
	)

	target_compile_options(icelibrarygencode
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	set_property(TARGET icelibrarygencode PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
endif()


if(BUILD_SHARED_LIBS)
	install(TARGETS icelibrarygencode DESTINATION test/bin)
else()
	install(TARGETS icelibrarygencode DESTINATION test/lib)
endif()

if(BUILD_ICE_CPP98)
	add_library(icelibraryconsumer)
	set_property(TARGET icelibraryconsumer
		PROPERTY
			OUTPUT_NAME consumer${zeroc-ice_VERSION_MAJOR}${zeroc-ice_VERSION_MINOR}
	)
elseif(BUILD_ICE_CPP11)
	add_library(icelibraryconsumer++11)
	set_property(TARGET icelibraryconsumer++11
		PROPERTY
			OUTPUT_NAME consumer${zeroc-ice_VERSION_MAJOR}${zeroc-ice_VERSION_MINOR}++11
	)
endif()

target_sources(icelibraryconsumer
	PRIVATE
		Consumer.cpp
)

target_include_directories(icelibraryconsumer
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/src
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/Ice
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET icelibraryconsumer PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET icelibraryconsumer PROPERTY CXX_STANDARD 11)
endif()

target_link_libraries(icelibraryconsumer 
	PRIVATE 
		${LIBRARY_DEPS}
		icelibrarygencode
)

target_compile_definitions(icelibraryconsumer PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	set_property(TARGET icelibraryconsumer PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
endif()

if(BUILD_SHARED_LIBS)
	install(TARGETS icelibraryconsumer DESTINATION test/bin)
else()
	install(TARGETS icelibraryconsumer DESTINATION test/lib)
endif()

if(BUILD_ICE_CPP98)
	add_library(icelibraryalltests)
	set_property(TARGET icelibraryalltests
		PROPERTY
			OUTPUT_NAME alltests${zeroc-ice_VERSION_MAJOR}${zeroc-ice_VERSION_MINOR}
	)
elseif(BUILD_ICE_CPP11)
	add_library(icelibraryalltests++11)
	set_property(TARGET icelibraryalltests++11
		PROPERTY
			OUTPUT_NAME alltests${zeroc-ice_VERSION_MAJOR}${zeroc-ice_VERSION_MINOR}++11
	)
endif()

target_sources(icelibraryalltests
	PRIVATE
		AllTests.cpp
)

target_include_directories(icelibraryalltests
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/src
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/Ice
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET icelibraryalltests PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET icelibraryalltests PROPERTY CXX_STANDARD 11)
endif()

target_link_libraries(icelibraryalltests 
	PRIVATE 
		${LIBRARY_DEPS}
		icelibraryconsumer
		icelibrarygencode
)

target_compile_definitions(icelibraryalltests PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	set_property(TARGET icelibraryalltests PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
endif()

if(BUILD_SHARED_LIBS)
	install(TARGETS icelibraryalltests DESTINATION test/bin)
else()
	install(TARGETS icelibraryalltests DESTINATION test/lib)
endif()

add_executable(icelibraryclient)

target_sources(icelibraryclient
	PRIVATE
		Client.cpp
)

target_include_directories(icelibraryclient
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/src
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/Ice
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET icelibraryclient PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET icelibraryclient PROPERTY CXX_STANDARD 11)
endif()

target_link_libraries(icelibraryclient 
	PRIVATE 
		${LIBRARY_DEPS}
		icelibraryalltests
)

target_compile_definitions(icelibraryclient PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	target_compile_options(icelibraryclient
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	set_property(TARGET icelibraryclient PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
endif()

install(TARGETS icelibraryclient DESTINATION tests/bin)
