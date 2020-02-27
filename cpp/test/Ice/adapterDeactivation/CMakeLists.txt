if(NOT ${CMAKE_CURRENT_BINARY_DIR}/Ice)
	file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Ice)
endif()

set(GENERATED_ADAPTERDEACT_HEADERS ${CMAKE_CURRENT_BINARY_DIR}/Ice/Test.h)
set(GENERATED_ADAPTERDEACT_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/Ice/Test.cpp)

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
	set(ADAPTERDEACT_DEPS 
		ice
		testcommon
	)
elseif(BUILD_ICE_CPP11)
	set(ADAPTERDEACT_DEPS 
		ice++11
		testcommon++11
	)
elseif(BUILD_ICE_UWP)
	set(ADAPTERDEACT_DEPS 
		iceuwp++11
		testcommonuwp++11
	)
endif()

add_executable(iceadapterdeactclient)

target_sources(iceadapterdeactclient
	PRIVATE
		AllTests.cpp
		Client.cpp
		${GENERATED_ADAPTERDEACT_HEADERS}
		${GENERATED_ADAPTERDEACT_SOURCES}
)

target_include_directories(iceadapterdeactclient
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/Ice
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET iceadapterdeactclient PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET iceadapterdeactclient PROPERTY CXX_STANDARD 11)
endif()

if(BUILD_ICE_UWP)
	add_dependencies(iceadapterdeactclient ${ADAPTERDEACT_DEPS})
	set_property(TARGET iceadapterdeactclient PROPERTY CXX_STANDARD 11)
else()
	target_link_libraries(iceadapterdeactclient PRIVATE ${ADAPTERDEACT_DEPS})
endif()

target_compile_definitions(iceadapterdeactclient PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Test.ice
	)

	source_group("Source Files/Generated"
		FILES
			${GENERATED_ADAPTERDEACT_SOURCES}
	)

	source_group("Header Files/Generated"
		FILES
			${GENERATED_ADAPTERDEACT_HEADERS}
	)

	target_compile_options(iceadapterdeactclient
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	if(NOT BUILD_ICE_UWP)
		set_property(TARGET iceadapterdeactclient PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
	else()
		set_property(TARGET iceadapterdeactclient PROPERTY LINK_FLAGS ${ICE_UWP_LINK_OPTIONS})
	endif()
endif()

install(TARGETS iceadapterdeactclient DESTINATION tests/bin)

add_executable(iceadapterdeactcollocated)

target_sources(iceadapterdeactcollocated
	PRIVATE
		AllTests.cpp
		Collocated.cpp
		ServantLocatorI.cpp
		TestI.cpp
		${GENERATED_ADAPTERDEACT_HEADERS}
		${GENERATED_ADAPTERDEACT_SOURCES}
)

target_include_directories(iceadapterdeactcollocated
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/Ice
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_UWP)
	add_dependencies(iceadapterdeactcollocated ${ADAPTERDEACT_DEPS})
else()
	target_link_libraries(iceadapterdeactcollocated PRIVATE ${ADAPTERDEACT_DEPS})
endif()

target_compile_definitions(iceadapterdeactcollocated PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Test.ice
	)

	source_group("Source Files/Generated"
		FILES
			${GENERATED_ADAPTERDEACT_SOURCES}
	)

	source_group("Header Files/Generated"
		FILES
			${GENERATED_ADAPTERDEACT_HEADERS}
	)

	target_compile_options(iceadapterdeactcollocated
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	if(NOT BUILD_ICE_UWP)
		set_property(TARGET iceadapterdeactcollocated PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
	else()
		set_property(TARGET iceadapterdeactcollocated PROPERTY LINK_FLAGS ${ICE_UWP_LINK_OPTIONS})
	endif()
endif()

install(TARGETS iceadapterdeactcollocated DESTINATION tests/bin)

add_executable(iceadapterdeactserver)

target_sources(iceadapterdeactserver
	PRIVATE
		Server.cpp
		ServantLocatorI.cpp
		TestI.cpp
		${GENERATED_ADAPTERDEACT_HEADERS}
		${GENERATED_ADAPTERDEACT_SOURCES}
)

target_include_directories(iceadapterdeactserver
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/Ice
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET iceadapterdeactserver PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET iceadapterdeactserver PROPERTY CXX_STANDARD 11)
endif()

if(BUILD_ICE_UWP)
	add_dependencies(iceadapterdeactserver ${ADAPTERDEACT_DEPS})
	set_property(TARGET iceadapterdeactserver PROPERTY CXX_STANDARD 11)
else()
	target_link_libraries(iceadapterdeactserver PRIVATE ${ADAPTERDEACT_DEPS})
endif()

target_compile_definitions(iceadapterdeactserver PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Test.ice
	)

	source_group("Source Files/Generated"
		FILES
			${GENERATED_ADAPTERDEACT_SOURCES}
	)

	source_group("Header Files/Generated"
		FILES
			${GENERATED_ADAPTERDEACT_HEADERS}
	)

	target_compile_options(iceadapterdeactserver
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	if(NOT BUILD_ICE_UWP)
		set_property(TARGET iceadapterdeactserver PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
	else()
		set_property(TARGET iceadapterdeactserver PROPERTY LINK_FLAGS ${ICE_UWP_LINK_OPTIONS})
	endif()
endif()

install(TARGETS iceadapterdeactserver DESTINATION tests/bin)
