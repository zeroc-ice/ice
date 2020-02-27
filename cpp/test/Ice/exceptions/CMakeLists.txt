if(NOT ${CMAKE_CURRENT_BINARY_DIR}/Ice)
	file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Ice)
endif()

set(EXCEPTIONS_SLICES
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	${CMAKE_CURRENT_SOURCE_DIR}/TestAMD.ice
)

set(GENERATED_EXCEPTIONS_HEADERS ${CMAKE_CURRENT_BINARY_DIR}/Ice/Test.h)
set(GENERATED_EXCEPTIONS_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/Ice/Test.cpp)

list(APPEND ICE_SLICE_DIRS "-I${SLICE_DIR}")
list(APPEND ICE_SLICE_PARAMS ${COMPILE_SLICES_CPP_PARAMETERS} "Ice")

foreach(slice ${EXCEPTIONS_SLICES})
	get_filename_component(slice_name ${slice} NAME_WE)
	set(output_files 
		${CMAKE_CURRENT_BINARY_DIR}/Ice/${slice_name}.cpp 
		${CMAKE_CURRENT_BINARY_DIR}/Ice/${slice_name}.h
	)	
	#function(CompileSlice slice2bin_params slice_include_paths ice_file output_dir output_file)
	CompileSlice("${ICE_SLICE_PARAMS}"
		"${ICE_SLICE_DIRS}"
		${slice}
		${CMAKE_CURRENT_BINARY_DIR}/Ice
		"${output_files}"
	)
endforeach()

if(BUILD_ICE_CPP98)
	set(EXCEPTIONS_DEPS 
		ice
		testcommon
	)
elseif(BUILD_ICE_CPP11)
	set(EXCEPTIONS_DEPS 
		ice++11
		testcommon++11
	)
elseif(BUILD_ICE_UWP)
	set(EXCEPTIONS_DEPS 
		iceuwp++11
		testcommonuwp++11
	)
endif()

add_executable(iceexceptionsclient)

target_sources(iceexceptionsclient
	PRIVATE
		AllTests.cpp
		Client.cpp
		ExceptionsI.cpp
		${CMAKE_CURRENT_BINARY_DIR}/Ice/Test.cpp
)

target_include_directories(iceexceptionsclient
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/Ice
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET iceexceptionsclient PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET iceexceptionsclient PROPERTY CXX_STANDARD 11)
endif()

if(BUILD_ICE_UWP)
	add_dependencies(iceexceptionsclient ${EXCEPTIONS_DEPS})
	set_property(TARGET iceexceptionsclient PROPERTY CXX_STANDARD 11)
else()
	target_link_libraries(iceexceptionsclient PRIVATE ${EXCEPTIONS_DEPS})
endif()

target_compile_definitions(iceexceptionsclient PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Test.ice
	)

	source_group("Source Files/Generated"
		FILES
			${CMAKE_CURRENT_BINARY_DIR}/Ice/Test.cpp
	)

	source_group("Header Files/Generated"
		FILES
			${CMAKE_CURRENT_BINARY_DIR}/Ice/Test.h
	)

	target_compile_options(iceexceptionsclient
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	if(NOT BUILD_ICE_UWP)
		set_property(TARGET iceexceptionsclient PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
	else()
		set_property(TARGET iceexceptionsclient PROPERTY LINK_FLAGS ${ICE_UWP_LINK_OPTIONS})
	endif()
endif()

install(TARGETS iceexceptionsclient DESTINATION tests/bin)

add_executable(iceexceptionscollocated)

target_sources(iceexceptionscollocated
	PRIVATE
		AllTests.cpp
		Collocated.cpp
		ExceptionsI.cpp
		TestI.cpp
		${CMAKE_CURRENT_BINARY_DIR}/Ice/Test.cpp
)

target_include_directories(iceexceptionscollocated
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/Ice
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET iceexceptionscollocated PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET iceexceptionscollocated PROPERTY CXX_STANDARD 11)
endif()

if(BUILD_ICE_UWP)
	add_dependencies(iceexceptionscollocated ${EXCEPTIONS_DEPS})
	set_property(TARGET iceexceptionscollocated PROPERTY CXX_STANDARD 11)
else()
	target_link_libraries(iceexceptionscollocated PRIVATE ${EXCEPTIONS_DEPS})
endif()

target_compile_definitions(iceexceptionscollocated PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Test.ice
	)

	source_group("Source Files/Generated"
		FILES
			${CMAKE_CURRENT_BINARY_DIR}/Ice/Test.cpp
	)

	source_group("Header Files/Generated"
		FILES
			${CMAKE_CURRENT_BINARY_DIR}/Ice/Test.h
	)

	target_compile_options(iceexceptionscollocated
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	if(NOT BUILD_ICE_UWP)
		set_property(TARGET iceexceptionscollocated PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
	else()
		set_property(TARGET iceexceptionscollocated PROPERTY LINK_FLAGS ${ICE_UWP_LINK_OPTIONS})
	endif()
endif()

install(TARGETS iceexceptionscollocated DESTINATION tests/bin)

add_executable(iceexceptionsserver)

target_sources(iceexceptionsserver
	PRIVATE
		ExceptionsI.cpp
		Server.cpp
		TestI.cpp
		${CMAKE_CURRENT_BINARY_DIR}/Ice/Test.cpp
)

target_include_directories(iceexceptionsserver
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/Ice
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET iceexceptionsserver PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET iceexceptionsserver PROPERTY CXX_STANDARD 11)
endif()

if(BUILD_ICE_UWP)
	add_dependencies(iceexceptionsserver ${EXCEPTIONS_DEPS})
	set_property(TARGET iceexceptionsserver PROPERTY CXX_STANDARD 11)
else()
	target_link_libraries(iceexceptionsserver PRIVATE ${EXCEPTIONS_DEPS})
endif()

target_compile_definitions(iceexceptionsserver PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Test.ice
	)

	source_group("Source Files/Generated"
		FILES
			${CMAKE_CURRENT_BINARY_DIR}/Ice/Test.cpp
	)

	source_group("Header Files/Generated"
		FILES
			${CMAKE_CURRENT_BINARY_DIR}/Ice/Test.h
	)

	target_compile_options(iceexceptionsserver
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	if(NOT BUILD_ICE_UWP)
		set_property(TARGET iceexceptionsserver PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
	else()
		set_property(TARGET iceexceptionsserver PROPERTY LINK_FLAGS ${ICE_UWP_LINK_OPTIONS})
	endif()
endif()

install(TARGETS iceexceptionsserver DESTINATION tests/bin)

add_executable(iceexceptionsserveramd)

target_sources(iceexceptionsserveramd
	PRIVATE
		ExceptionsI.cpp
		ServerAMD.cpp
		TestAMDI.cpp
		${CMAKE_CURRENT_BINARY_DIR}/Ice/TestAMD.cpp
)

target_include_directories(iceexceptionsserveramd
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/Ice
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET iceexceptionsserveramd PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET iceexceptionsserveramd PROPERTY CXX_STANDARD 11)
endif()

if(BUILD_ICE_UWP)
	add_dependencies(iceexceptionsserveramd ${EXCEPTIONS_DEPS})
	set_property(TARGET iceexceptionsserveramd PROPERTY CXX_STANDARD 11)
else()
	target_link_libraries(iceexceptionsserveramd PRIVATE ${EXCEPTIONS_DEPS})
endif()

target_compile_definitions(iceexceptionsserveramd 
	PRIVATE 
		${ICE_COMPILE_DEFS}
		"ICE_AMD_TEST"
)

if(MSVC)
	source_group("Slice Files"
		FILES
			TestAMD.ice
	)

	source_group("Source Files/Generated"
		FILES
			${CMAKE_CURRENT_BINARY_DIR}/Ice/TestAMD.cpp
	)

	source_group("Header Files/Generated"
		FILES
			${CMAKE_CURRENT_BINARY_DIR}/Ice/TestAMD.h
	)

	target_compile_options(iceexceptionsserveramd
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	if(NOT BUILD_ICE_UWP)
		set_property(TARGET iceexceptionsserveramd PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
	else()
		set_property(TARGET iceexceptionsserveramd PROPERTY LINK_FLAGS ${ICE_UWP_LINK_OPTIONS})
	endif()
endif()

install(TARGETS iceexceptionsserveramd DESTINATION tests/bin)
