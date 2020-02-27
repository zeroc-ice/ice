if(NOT ${CMAKE_CURRENT_BINARY_DIR}/Ice)
	file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Ice)
endif()

set(OPERATIONS_SLICES
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	${CMAKE_CURRENT_SOURCE_DIR}/TestAMD.ice
)

list(APPEND ICE_SLICE_DIRS "-I${SLICE_DIR}")
list(APPEND ICE_SLICE_PARAMS ${COMPILE_SLICES_CPP_PARAMETERS} "Ice")

foreach(slice ${OPERATIONS_SLICES})
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
	set(OPERATIONS_DEPS 
		ice
		testcommon
	)
elseif(BUILD_ICE_CPP11)
	set(OPERATIONS_DEPS 
		ice++11
		testcommon++11
	)
elseif(BUILD_ICE_UWP)
	set(OPERATIONS_DEPS 
		iceuwp++11
		testcommonuwp++11
	)
endif()

add_executable(iceoperationsclient)

target_sources(iceoperationsclient
	PRIVATE
		AllTests.cpp
		BatchOneways.cpp
		BatchOnewaysAMI.cpp
		Client.cpp
		Oneways.cpp
		OnewaysAMI.cpp
		Twoways.cpp
		TwowaysAMI.cpp
		${CMAKE_CURRENT_BINARY_DIR}/Ice/Test.cpp
)

target_include_directories(iceoperationsclient
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/Ice
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET iceoperationsclient PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET iceoperationsclient PROPERTY CXX_STANDARD 11)
endif()

if(BUILD_ICE_UWP)
	add_dependencies(iceoperationsclient ${OPERATIONS_DEPS})
	set_property(TARGET iceoperationsclient PROPERTY CXX_STANDARD 11)
else()
	target_link_libraries(iceoperationsclient PRIVATE ${OPERATIONS_DEPS})
endif()

target_compile_definitions(iceoperationsclient PRIVATE ${ICE_COMPILE_DEFS})

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

	target_compile_options(iceoperationsclient
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
			"/wd4503"
	)

	if(NOT BUILD_ICE_UWP)
		set_property(TARGET iceoperationsclient PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
	else()
		set_property(TARGET iceoperationsclient PROPERTY LINK_FLAGS ${ICE_UWP_LINK_OPTIONS})
	endif()
endif()

install(TARGETS iceoperationsclient DESTINATION tests/bin)

add_executable(iceoperationscollocated)

target_sources(iceoperationscollocated
	PRIVATE
		AllTests.cpp
		BatchOneways.cpp
		BatchOnewaysAMI.cpp
		Collocated.cpp
		Oneways.cpp
		OnewaysAMI.cpp
		TestI.cpp
		Twoways.cpp
		TwowaysAMI.cpp
		${CMAKE_CURRENT_BINARY_DIR}/Ice/Test.cpp
)

target_include_directories(iceoperationscollocated
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/Ice
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET iceoperationscollocated PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET iceoperationscollocated PROPERTY CXX_STANDARD 11)
endif()

if(BUILD_ICE_UWP)
	add_dependencies(iceoperationscollocated ${OPERATIONS_DEPS})
	set_property(TARGET iceoperationscollocated PROPERTY CXX_STANDARD 11)
else()
	target_link_libraries(iceoperationscollocated PRIVATE ${OPERATIONS_DEPS})
endif()

target_compile_definitions(iceoperationscollocated PRIVATE ${ICE_COMPILE_DEFS})

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

	target_compile_options(iceoperationscollocated
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
			"/wd4503"
	)

	if(NOT BUILD_ICE_UWP)
		set_property(TARGET iceoperationscollocated PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
	else()
		set_property(TARGET iceoperationscollocated PROPERTY LINK_FLAGS ${ICE_UWP_LINK_OPTIONS})
	endif()
endif()

install(TARGETS iceoperationscollocated DESTINATION tests/bin)

add_executable(iceoperationsserver)

target_sources(iceoperationsserver
	PRIVATE
		Server.cpp
		TestI.cpp
		${CMAKE_CURRENT_BINARY_DIR}/Ice/Test.cpp
)

target_include_directories(iceoperationsserver
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/Ice
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET iceoperationsserver PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET iceoperationsserver PROPERTY CXX_STANDARD 11)
endif()

if(BUILD_ICE_UWP)
	add_dependencies(iceoperationsserver ${OPERATIONS_DEPS})
	set_property(TARGET iceoperationsserver PROPERTY CXX_STANDARD 11)
else()
	target_link_libraries(iceoperationsserver PRIVATE ${OPERATIONS_DEPS})
endif()

target_compile_definitions(iceoperationsserver PRIVATE ${ICE_COMPILE_DEFS})

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

	target_compile_options(iceoperationsserver
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
			"/wd4503"
	)

	if(NOT BUILD_ICE_UWP)
		set_property(TARGET iceoperationsserver PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
	else()
		set_property(TARGET iceoperationsserver PROPERTY LINK_FLAGS ${ICE_UWP_LINK_OPTIONS})
	endif()
endif()

install(TARGETS iceoperationsserver DESTINATION tests/bin)

add_executable(iceoperationsserveramd)

target_sources(iceoperationsserveramd
	PRIVATE
		ServerAMD.cpp
		TestAMDI.cpp
		${CMAKE_CURRENT_BINARY_DIR}/Ice/TestAMD.cpp
)

target_include_directories(iceoperationsserveramd
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/Ice
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET iceoperationsserveramd PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET iceoperationsserveramd PROPERTY CXX_STANDARD 11)
endif()

if(BUILD_ICE_UWP)
	add_dependencies(iceoperationsserveramd ${OPERATIONS_DEPS})
	set_property(TARGET iceoperationsserveramd PROPERTY CXX_STANDARD 11)
else()
	target_link_libraries(iceoperationsserveramd PRIVATE ${OPERATIONS_DEPS})
endif()

target_compile_definitions(iceoperationsserveramd 
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

	target_compile_options(iceoperationsserveramd
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
			"/wd4503"
	)

	if(NOT BUILD_ICE_UWP)
		set_property(TARGET iceoperationsserveramd PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
	else()
		set_property(TARGET iceoperationsserveramd PROPERTY LINK_FLAGS ${ICE_UWP_LINK_OPTIONS})
	endif()
endif()

install(TARGETS iceoperationsserveramd DESTINATION tests/bin)
