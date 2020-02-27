if(NOT ${CMAKE_CURRENT_BINARY_DIR}/Ice)
	file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Ice)
endif()

set(GENERATED_SERVICES_HEADERS ${CMAKE_CURRENT_BINARY_DIR}/Ice/Test.h)
set(GENERATED_SERVICES_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/Ice/Test.cpp)

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
	set(SERVICES_DEPS 
		icegrid
		icestorm
		ice
		glacier2
		testcommon
	)
elseif(BUILD_ICE_CPP11)
	set(SERVICES_DEPS 
		icegrid++11
		icestorm++11
		ice++11
		glacier2++11
		testcommon++11
	)
elseif(BUILD_ICE_UWP)
	set(SERVICES_DEPS 
		icegriduwp++11
		icestormuwp++11
		iceuwp++11
		glacier2uwp++11
		testcommonuwp++11
	)
endif()

add_executable(iceservicesclient)

target_sources(iceservicesclient
	PRIVATE
		AllTests.cpp
		Client.cpp
		${GENERATED_SERVICES_HEADERS}
		${GENERATED_SERVICES_SOURCES}
)

target_include_directories(iceservicesclient
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/Ice
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET iceservicesclient PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET iceservicesclient PROPERTY CXX_STANDARD 11)
endif()

if(BUILD_ICE_UWP)
	add_dependencies(iceservicesclient ${SERVICES_DEPS})
	set_property(TARGET iceservicesclient PROPERTY CXX_STANDARD 11)
else()
	target_link_libraries(iceservicesclient PRIVATE ${SERVICES_DEPS})
endif()

target_compile_definitions(iceservicesclient PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Test.ice
	)

	source_group("Source Files/Generated"
		FILES
			${GENERATED_SERVICES_SOURCES}
	)

	source_group("Header Files/Generated"
		FILES
			${GENERATED_SERVICES_HEADERS}
	)

	target_compile_options(iceservicesclient
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
			"/wd4503"
	)

	if(NOT BUILD_ICE_UWP)
		set_property(TARGET iceservicesclient PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
	else()
		set_property(TARGET iceservicesclient PROPERTY LINK_FLAGS ${ICE_UWP_LINK_OPTIONS})
	endif()
endif()

install(TARGETS iceservicesclient DESTINATION tests/bin)
