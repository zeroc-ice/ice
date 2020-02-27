if(NOT ${CMAKE_CURRENT_BINARY_DIR}/Ice)
	file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Ice)
endif()

set(GENERATED_AMI_HEADERS ${CMAKE_CURRENT_BINARY_DIR}/Ice/Test.h)
set(GENERATED_AMI_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/Ice/Test.cpp)

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
	set(AMI_DEPS 
		ice
		testcommon
	)
elseif(BUILD_ICE_CPP11)
	set(AMI_DEPS 
		ice++11
		testcommon++11
	)
elseif(BUILD_ICE_UWP)
	set(AMI_DEPS 
		iceuwp++11
		testcommonuwp++11
	)
endif()

add_executable(iceamiclient)

target_sources(iceamiclient
	PRIVATE
		AllTests.cpp
		Client.cpp
		${GENERATED_AMI_HEADERS}
		${GENERATED_AMI_SOURCES}
)

target_include_directories(iceamiclient
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/Ice
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET iceamiclient PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET iceamiclient PROPERTY CXX_STANDARD 11)
endif()

if(BUILD_ICE_UWP)
	add_dependencies(iceamiclient ${AMI_DEPS})
	set_property(TARGET iceamiclient PROPERTY CXX_STANDARD 11)
else()
	target_link_libraries(iceamiclient PRIVATE ${AMI_DEPS})
endif()

target_compile_definitions(iceamiclient PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Test.ice
	)

	source_group("Source Files/Generated"
		FILES
			${GENERATED_AMI_SOURCES}
	)

	source_group("Header Files/Generated"
		FILES
			${GENERATED_AMI_HEADERS}
	)

	target_compile_options(iceamiclient
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	if(NOT BUILD_ICE_UWP)
		set_property(TARGET iceamiclient PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
	else()
		set_property(TARGET iceamiclient PROPERTY LINK_FLAGS ${ICE_UWP_LINK_OPTIONS})
	endif()
endif()

install(TARGETS iceamiclient DESTINATION tests/bin)

add_executable(iceamicollocated)

target_sources(iceamicollocated
	PRIVATE
		AllTests.cpp
		Collocated.cpp
		TestI.cpp
		${GENERATED_AMI_HEADERS}
		${GENERATED_AMI_SOURCES}
)

target_include_directories(iceamicollocated
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/Ice
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET iceamicollocated PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET iceamicollocated PROPERTY CXX_STANDARD 11)
endif()

if(BUILD_ICE_UWP)
	add_dependencies(iceamicollocated ${AMI_DEPS})
	set_property(TARGET iceamicollocated PROPERTY CXX_STANDARD 11)
else()
	target_link_libraries(iceamicollocated PRIVATE ${AMI_DEPS})
endif()

target_compile_definitions(iceamicollocated PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Test.ice
	)

	source_group("Source Files/Generated"
		FILES
			${GENERATED_AMI_SOURCES}
	)

	source_group("Header Files/Generated"
		FILES
			${GENERATED_AMI_HEADERS}
	)

	target_compile_options(iceamicollocated
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	if(NOT BUILD_ICE_UWP)
		set_property(TARGET iceamicollocated PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
	else()
		set_property(TARGET iceamicollocated PROPERTY LINK_FLAGS ${ICE_UWP_LINK_OPTIONS})
	endif()
endif()

install(TARGETS iceamicollocated DESTINATION tests/bin)

add_executable(iceamiserver)

target_sources(iceamiserver
	PRIVATE
		Server.cpp
		TestI.cpp
		${GENERATED_AMI_HEADERS}
		${GENERATED_AMI_SOURCES}
)

target_include_directories(iceamiserver
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/Ice
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET iceamiserver PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET iceamiserver PROPERTY CXX_STANDARD 11)
endif()

if(BUILD_ICE_UWP)
	add_dependencies(iceamiserver ${AMI_DEPS})
	set_property(TARGET iceamiserver PROPERTY CXX_STANDARD 11)
else()
	target_link_libraries(iceamiserver PRIVATE ${AMI_DEPS})
endif()

target_compile_definitions(iceamiserver PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Test.ice
	)

	source_group("Source Files/Generated"
		FILES
			${GENERATED_AMI_SOURCES}
	)

	source_group("Header Files/Generated"
		FILES
			${GENERATED_AMI_HEADERS}
	)

	target_compile_options(iceamiserver
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	if(NOT BUILD_ICE_UWP)
		set_property(TARGET iceamiserver PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
	else()
		set_property(TARGET iceamiserver PROPERTY LINK_FLAGS ${ICE_UWP_LINK_OPTIONS})
	endif()
endif()

install(TARGETS iceamiserver DESTINATION tests/bin)
