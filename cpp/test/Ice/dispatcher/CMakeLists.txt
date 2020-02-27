if(NOT ${CMAKE_CURRENT_BINARY_DIR}/Ice)
	file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Ice)
endif()

set(GENERATED_DISPATCHER_HEADERS ${CMAKE_CURRENT_BINARY_DIR}/Ice/Test.h)
set(GENERATED_DISPATCHER_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/Ice/Test.cpp)

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
	set(DISPATCHER_DEPS 
		ice
		testcommon
	)
elseif(BUILD_ICE_CPP11)
	set(DISPATCHER_DEPS 
		ice++11
		testcommon++11
	)
elseif(BUILD_ICE_UWP)
	set(DISPATCHER_DEPS 
		iceuwp++11
		testcommonuwp++11
	)
endif()

add_executable(icedispatcherclient)

target_sources(icedispatcherclient
	PRIVATE
		AllTests.cpp
		Client.cpp
		Dispatcher.cpp
		${GENERATED_DISPATCHER_HEADERS}
		${GENERATED_DISPATCHER_SOURCES}
)

target_include_directories(icedispatcherclient
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/Ice
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET icedispatcherclient PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET icedispatcherclient PROPERTY CXX_STANDARD 11)
endif()

if(BUILD_ICE_UWP)
	add_dependencies(icedispatcherclient ${DISPATCHER_DEPS})
	set_property(TARGET icedispatcherclient PROPERTY CXX_STANDARD 11)
else()
	target_link_libraries(icedispatcherclient PRIVATE ${DISPATCHER_DEPS})
endif()

target_compile_definitions(icedispatcherclient PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Test.ice
	)

	source_group("Source Files/Generated"
		FILES
			${GENERATED_DISPATCHER_SOURCES}
	)

	source_group("Header Files/Generated"
		FILES
			${GENERATED_DISPATCHER_HEADERS}
	)

	target_compile_options(icedispatcherclient
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	if(NOT BUILD_ICE_UWP)
		set_property(TARGET icedispatcherclient PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
	else()
		set_property(TARGET icedispatcherclient PROPERTY LINK_FLAGS ${ICE_UWP_LINK_OPTIONS})
	endif()
endif()

install(TARGETS icedispatcherclient DESTINATION tests/bin)

add_executable(icedispatchercollocated)

target_sources(icedispatchercollocated
	PRIVATE
		AllTests.cpp
		Collocated.cpp
		Dispatcher.cpp
		TestI.cpp
		${GENERATED_DISPATCHER_HEADERS}
		${GENERATED_DISPATCHER_SOURCES}
)

target_include_directories(icedispatchercollocated
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/Ice
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET icedispatchercollocated PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET icedispatchercollocated PROPERTY CXX_STANDARD 11)
endif()

if(BUILD_ICE_UWP)
	add_dependencies(icedispatchercollocated ${DISPATCHER_DEPS})
	set_property(TARGET icedispatchercollocated PROPERTY CXX_STANDARD 11)
else()
	target_link_libraries(icedispatchercollocated PRIVATE ${DISPATCHER_DEPS})
endif()

target_compile_definitions(icedispatchercollocated PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Test.ice
	)

	source_group("Source Files/Generated"
		FILES
			${GENERATED_DISPATCHER_SOURCES}
	)

	source_group("Header Files/Generated"
		FILES
			${GENERATED_DISPATCHER_HEADERS}
	)

	target_compile_options(icedispatchercollocated
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	if(NOT BUILD_ICE_UWP)
		set_property(TARGET icedispatchercollocated PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
	else()
		set_property(TARGET icedispatchercollocated PROPERTY LINK_FLAGS ${ICE_UWP_LINK_OPTIONS})
	endif()
endif()

install(TARGETS icedispatchercollocated DESTINATION tests/bin)

add_executable(icedispatcherserver)

target_sources(icedispatcherserver
	PRIVATE
		Dispatcher.cpp
		Server.cpp
		TestI.cpp
		${GENERATED_DISPATCHER_HEADERS}
		${GENERATED_DISPATCHER_SOURCES}
)

target_include_directories(icedispatcherserver
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/Ice
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET icedispatcherserver PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET icedispatcherserver PROPERTY CXX_STANDARD 11)
endif()

if(BUILD_ICE_UWP)
	add_dependencies(icedispatcherserver ${DISPATCHER_DEPS})
	set_property(TARGET icedispatcherserver PROPERTY CXX_STANDARD 11)
else()
	target_link_libraries(icedispatcherserver PRIVATE ${DISPATCHER_DEPS})
endif()

target_compile_definitions(icedispatcherserver PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Test.ice
	)

	source_group("Source Files/Generated"
		FILES
			${GENERATED_DISPATCHER_SOURCES}
	)

	source_group("Header Files/Generated"
		FILES
			${GENERATED_DISPATCHER_HEADERS}
	)

	target_compile_options(icedispatcherserver
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	if(NOT BUILD_ICE_UWP)
		set_property(TARGET icedispatcherserver PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
	else()
		set_property(TARGET icedispatcherserver PROPERTY LINK_FLAGS ${ICE_UWP_LINK_OPTIONS})
	endif()
endif()

install(TARGETS icedispatcherserver DESTINATION tests/bin)
