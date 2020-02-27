if(NOT ${CMAKE_CURRENT_BINARY_DIR}/Ice)
	file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Ice)
endif()

set(GENERATED_INVOKE_HEADERS ${CMAKE_CURRENT_BINARY_DIR}/Ice/Test.h)
set(GENERATED_INVOKE_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/Ice/Test.cpp)

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
	set(INVOKE_DEPS 
		ice
		testcommon
	)
elseif(BUILD_ICE_CPP11)
	set(INVOKE_DEPS 
		ice++11
		testcommon++11
	)
elseif(BUILD_ICE_UWP)
	set(INVOKE_DEPS 
		iceuwp++11
		testcommonuwp++11
	)
endif()

add_executable(iceinvokeclient)

target_sources(iceinvokeclient
	PRIVATE
		AllTests.cpp
		Client.cpp
		${GENERATED_INVOKE_HEADERS}
		${GENERATED_INVOKE_SOURCES}
)

target_include_directories(iceinvokeclient
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/Ice
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET iceinvokeclient PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET iceinvokeclient PROPERTY CXX_STANDARD 11)
endif()

if(BUILD_ICE_UWP)
	add_dependencies(iceinvokeclient ${INVOKE_DEPS})
	set_property(TARGET iceinvokeclient PROPERTY CXX_STANDARD 11)
else()
	target_link_libraries(iceinvokeclient PRIVATE ${INVOKE_DEPS})
endif()

target_compile_definitions(iceinvokeclient PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Test.ice
	)

	source_group("Source Files/Generated"
		FILES
			${GENERATED_INVOKE_SOURCES}
	)

	source_group("Header Files/Generated"
		FILES
			${GENERATED_INVOKE_HEADERS}
	)

	target_compile_options(iceinvokeclient
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	if(NOT BUILD_ICE_UWP)
		set_property(TARGET iceinvokeclient PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
	else()
		set_property(TARGET iceinvokeclient PROPERTY LINK_FLAGS ${ICE_UWP_LINK_OPTIONS})
	endif()
endif()

install(TARGETS iceinvokeclient DESTINATION tests/bin)

add_executable(iceinvokeserver)

target_sources(iceinvokeserver
	PRIVATE
		BlobjectI.cpp
		Server.cpp
		${GENERATED_INVOKE_HEADERS}
		${GENERATED_INVOKE_SOURCES}
)

target_include_directories(iceinvokeserver
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/Ice
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET iceinvokeserver PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET iceinvokeserver PROPERTY CXX_STANDARD 11)
endif()

if(BUILD_ICE_UWP)
	add_dependencies(iceinvokeserver ${INVOKE_DEPS})
	set_property(TARGET iceinvokeserver PROPERTY CXX_STANDARD 11)
else()
	target_link_libraries(iceinvokeserver PRIVATE ${INVOKE_DEPS})
endif()

target_compile_definitions(iceinvokeserver PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Test.ice
	)

	source_group("Source Files/Generated"
		FILES
			${GENERATED_INVOKE_SOURCES}
	)

	source_group("Header Files/Generated"
		FILES
			${GENERATED_INVOKE_HEADERS}
	)

	target_compile_options(iceinvokeserver
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	if(NOT BUILD_ICE_UWP)
		set_property(TARGET iceinvokeserver PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
	else()
		set_property(TARGET iceinvokeserver PROPERTY LINK_FLAGS ${ICE_UWP_LINK_OPTIONS})
	endif()
endif()

install(TARGETS iceinvokeserver DESTINATION tests/bin)
