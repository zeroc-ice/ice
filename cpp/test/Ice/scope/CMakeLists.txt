if(NOT ${CMAKE_CURRENT_BINARY_DIR}/Ice)
	file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Ice)
endif()

set(GENERATED_SCOPE_HEADERS ${CMAKE_CURRENT_BINARY_DIR}/Ice/Test.h)
set(GENERATED_SCOPE_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/Ice/Test.cpp)

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
	set(SCOPE_DEPS 
		ice
		testcommon
	)
elseif(BUILD_ICE_CPP11)
	set(SCOPE_DEPS 
		ice++11
		testcommon++11
	)
elseif(BUILD_ICE_UWP)
	set(SCOPE_DEPS 
		iceuwp++11
		testcommonuwp++11
	)
endif()

add_executable(icescopeclient)

target_sources(icescopeclient
	PRIVATE
		AllTests.cpp
		Client.cpp
		${GENERATED_SCOPE_HEADERS}
		${GENERATED_SCOPE_SOURCES}
)

target_include_directories(icescopeclient
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/Ice
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET icescopeclient PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET icescopeclient PROPERTY CXX_STANDARD 11)
endif()

if(BUILD_ICE_UWP)
	add_dependencies(icescopeclient ${SCOPE_DEPS})
	set_property(TARGET icescopeclient PROPERTY CXX_STANDARD 11)
else()
	target_link_libraries(icescopeclient PRIVATE ${SCOPE_DEPS})
endif()

target_compile_definitions(icescopeclient PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Test.ice
	)

	source_group("Source Files/Generated"
		FILES
			${GENERATED_SCOPE_SOURCES}
	)

	source_group("Header Files/Generated"
		FILES
			${GENERATED_SCOPE_HEADERS}
	)

	target_compile_options(icescopeclient
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
			"/wd4503"
	)

	if(NOT BUILD_ICE_UWP)
		set_property(TARGET icescopeclient PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
	else()
		set_property(TARGET icescopeclient PROPERTY LINK_FLAGS ${ICE_UWP_LINK_OPTIONS})
	endif()
endif()

install(TARGETS icescopeclient DESTINATION tests/bin)

add_executable(icescopeserver)

target_sources(icescopeserver
	PRIVATE
		Server.cpp
		${GENERATED_SCOPE_HEADERS}
		${GENERATED_SCOPE_SOURCES}
)

target_include_directories(icescopeserver
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/Ice
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET icescopeserver PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET icescopeserver PROPERTY CXX_STANDARD 11)
endif()

if(BUILD_ICE_UWP)
	add_dependencies(icescopeserver ${SCOPE_DEPS})
	set_property(TARGET icescopeserver PROPERTY CXX_STANDARD 11)
else()
	target_link_libraries(icescopeserver PRIVATE ${SCOPE_DEPS})
endif()

target_compile_definitions(icescopeserver PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Test.ice
	)

	source_group("Source Files/Generated"
		FILES
			${GENERATED_SCOPE_SOURCES}
	)

	source_group("Header Files/Generated"
		FILES
			${GENERATED_SCOPE_HEADERS}
	)

	target_compile_options(icescopeserver
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
			"/wd4503"
	)

	if(NOT BUILD_ICE_UWP)
		set_property(TARGET icescopeserver PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
	else()
		set_property(TARGET icescopeserver PROPERTY LINK_FLAGS ${ICE_UWP_LINK_OPTIONS})
	endif()
endif()

install(TARGETS icescopeserver DESTINATION tests/bin)
