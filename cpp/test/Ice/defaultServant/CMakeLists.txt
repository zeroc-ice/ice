if(NOT ${CMAKE_CURRENT_BINARY_DIR}/Ice)
	file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Ice)
endif()

set(GENERATED_DEFSERVANT_HEADERS ${CMAKE_CURRENT_BINARY_DIR}/Ice/Test.h)
set(GENERATED_DEFSERVANT_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/Ice/Test.cpp)

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
	set(DEFSERVANT_DEPS 
		ice
		testcommon
	)
elseif(BUILD_ICE_CPP11)
	set(DEFSERVANT_DEPS 
		ice++11
		testcommon++11
	)
elseif(BUILD_ICE_UWP)
	set(DEFSERVANT_DEPS 
		iceuwp++11
		testcommonuwp++11
	)
endif()

add_executable(icedefservantclient)

target_sources(icedefservantclient
	PRIVATE
		AllTests.cpp
		Client.cpp
		TestI.cpp
		${GENERATED_DEFSERVANT_HEADERS}
		${GENERATED_DEFSERVANT_SOURCES}
)

target_include_directories(icedefservantclient
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/Ice
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET icedefservantclient PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET icedefservantclient PROPERTY CXX_STANDARD 11)
endif()

if(BUILD_ICE_UWP)
	add_dependencies(icedefservantclient ${DEFSERVANT_DEPS})
	set_property(TARGET icedefservantclient PROPERTY CXX_STANDARD 11)
else()
	target_link_libraries(icedefservantclient PRIVATE ${DEFSERVANT_DEPS})
endif()

target_compile_definitions(icedefservantclient PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Test.ice
	)

	source_group("Source Files/Generated"
		FILES
			${GENERATED_DEFSERVANT_SOURCES}
	)

	source_group("Header Files/Generated"
		FILES
			${GENERATED_DEFSERVANT_HEADERS}
	)

	target_compile_options(icedefservantclient
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	if(NOT BUILD_ICE_UWP)
		set_property(TARGET icedefservantclient PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
	else()
		set_property(TARGET icedefservantclient PROPERTY LINK_FLAGS ${ICE_UWP_LINK_OPTIONS})
	endif()
endif()

install(TARGETS icedefservantclient DESTINATION tests/bin)
