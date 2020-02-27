if(NOT ${CMAKE_CURRENT_BINARY_DIR}/Ice)
	file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Ice)
endif()

set(GENERATED_RETRY_HEADERS ${CMAKE_CURRENT_BINARY_DIR}/Ice/Test.h)

set(GENERATED_RETRY_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/Ice/Test.cpp)

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
	set(RETRY_DEPS 
		ice
		testcommon
	)
elseif(BUILD_ICE_CPP11)
	set(RETRY_DEPS 
		ice++11
		testcommon++11
	)
elseif(BUILD_ICE_UWP)
	set(RETRY_DEPS 
		iceuwp++11
		testcommonuwp++11
	)
endif()

add_executable(iceretryclient)

target_sources(iceretryclient
	PRIVATE
		AllTests.cpp
		Client.cpp
		InstrumentationI.cpp
		${GENERATED_RETRY_HEADERS}
		${GENERATED_RETRY_SOURCES}
)

target_include_directories(iceretryclient
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/Ice
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET iceretryclient PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET iceretryclient PROPERTY CXX_STANDARD 11)
endif()

if(BUILD_ICE_UWP)
	add_dependencies(iceretryclient ${RETRY_DEPS})
	set_property(TARGET iceretryclient PROPERTY CXX_STANDARD 11)
else()
	target_link_libraries(iceretryclient PRIVATE ${RETRY_DEPS})
endif()

target_compile_definitions(iceretryclient PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Test.ice
	)

	source_group("Source Files/Generated"
		FILES
			${GENERATED_RETRY_SOURCES}
	)

	source_group("Header Files/Generated"
		FILES
			${GENERATED_RETRY_HEADERS}
	)

	target_compile_options(iceretryclient
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	if(NOT BUILD_ICE_UWP)
		set_property(TARGET iceretryclient 
			PROPERTY 
				LINK_FLAGS "${ICE_MSVC_DLL_LINK_OPTIONS} /STACK:8388608")
	else()
		set_property(TARGET iceretryclient PROPERTY LINK_FLAGS ${ICE_UWP_LINK_OPTIONS})
	endif()
endif()

install(TARGETS iceretryclient DESTINATION tests/bin)

add_executable(iceretrycollocated)

target_sources(iceretrycollocated
	PRIVATE
		AllTests.cpp
		Collocated.cpp
		InstrumentationI.cpp
		TestI.cpp
		${GENERATED_RETRY_HEADERS}
		${GENERATED_RETRY_SOURCES}
)

target_include_directories(iceretrycollocated
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/Ice
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_UWP)
	add_dependencies(iceretrycollocated ${RETRY_DEPS})
else()
	target_link_libraries(iceretrycollocated PRIVATE ${RETRY_DEPS})
endif()

target_compile_definitions(iceretrycollocated PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Test.ice
	)

	source_group("Source Files/Generated"
		FILES
			${GENERATED_RETRY_SOURCES}
	)

	source_group("Header Files/Generated"
		FILES
			${GENERATED_RETRY_HEADERS}
	)

	target_compile_options(iceretrycollocated
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	if(NOT BUILD_ICE_UWP)
		set_property(TARGET iceretrycollocated PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
	else()
		set_property(TARGET iceretrycollocated PROPERTY LINK_FLAGS ${ICE_UWP_LINK_OPTIONS})
	endif()
endif()

install(TARGETS iceretrycollocated DESTINATION tests/bin)

add_executable(iceretryserver)

target_sources(iceretryserver
	PRIVATE
		InstrumentationI.cpp
		Server.cpp
		TestI.cpp
		${GENERATED_RETRY_HEADERS}
		${GENERATED_RETRY_SOURCES}
)

target_include_directories(iceretryserver
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/Ice
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET iceretryserver PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET iceretryserver PROPERTY CXX_STANDARD 11)
endif()

if(BUILD_ICE_UWP)
	add_dependencies(iceretryserver ${RETRY_DEPS})
	set_property(TARGET iceretryserver PROPERTY CXX_STANDARD 11)
else()
	target_link_libraries(iceretryserver PRIVATE ${RETRY_DEPS})
endif()

target_compile_definitions(iceretryserver PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Test.ice
	)

	source_group("Source Files/Generated"
		FILES
			${GENERATED_RETRY_SOURCES}
	)

	source_group("Header Files/Generated"
		FILES
			${GENERATED_RETRY_HEADERS}
	)

	target_compile_options(iceretryserver
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	if(NOT BUILD_ICE_UWP)
		set_property(TARGET iceretryserver PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
	else()
		set_property(TARGET iceretryserver PROPERTY LINK_FLAGS ${ICE_UWP_LINK_OPTIONS})
	endif()
endif()

install(TARGETS iceretryserver DESTINATION tests/bin)
