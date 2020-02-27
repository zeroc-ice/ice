if(NOT ${CMAKE_CURRENT_BINARY_DIR}/Ice)
	file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Ice)
endif()

set(CHECKSUM_SLICES
	${CMAKE_CURRENT_SOURCE_DIR}/ClientTypes.ice
	${CMAKE_CURRENT_SOURCE_DIR}/ServerTypes.ice
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
)

list(APPEND ICE_SLICE_DIRS "-I${SLICE_DIR}")
list(APPEND ICE_SLICE_PARAMS ${COMPILE_SLICES_CPP_PARAMETERS} "Ice")

foreach(slice ${CHECKSUM_SLICES})
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
	set(CHECKSUM_DEPS 
		ice
		testcommon
	)
elseif(BUILD_ICE_CPP11)
	set(CHECKSUM_DEPS 
		ice++11
		testcommon++11
	)
elseif(BUILD_ICE_UWP)
	set(CHECKSUM_DEPS 
		iceuwp++11
		testcommonuwp++11
	)
endif()

add_executable(icechecksumclient)

target_sources(icechecksumclient
	PRIVATE
		AllTests.cpp
		Client.cpp
		${CMAKE_CURRENT_BINARY_DIR}/Ice/ClientTypes.cpp
		${CMAKE_CURRENT_BINARY_DIR}/Ice/Test.cpp
)

target_include_directories(icechecksumclient
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/Ice
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET icechecksumclient PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET icechecksumclient PROPERTY CXX_STANDARD 11)
endif()

if(BUILD_ICE_UWP)
	add_dependencies(icechecksumclient ${CHECKSUM_DEPS})
	set_property(TARGET icechecksumclient PROPERTY CXX_STANDARD 11)
else()
	target_link_libraries(icechecksumclient PRIVATE ${CHECKSUM_DEPS})
endif()

target_compile_definitions(icechecksumclient PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			ClientTypes.ice
			Test.ice
	)

	source_group("Source Files/Generated"
		FILES
			${CMAKE_CURRENT_BINARY_DIR}/Ice/ClientTypes.cpp
	)

	source_group("Header Files/Generated"
		FILES
			${CMAKE_CURRENT_BINARY_DIR}/Ice/ClientTypes.h
	)

	target_compile_options(icechecksumclient
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	if(NOT BUILD_ICE_UWP)
		set_property(TARGET icechecksumclient PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
	else()
		set_property(TARGET icechecksumclient PROPERTY LINK_FLAGS ${ICE_UWP_LINK_OPTIONS})
	endif()
endif()

install(TARGETS icechecksumclient DESTINATION tests/bin)

add_executable(icechecksumserver)

target_sources(icechecksumserver
	PRIVATE
		Server.cpp
		TestI.cpp
		${CMAKE_CURRENT_BINARY_DIR}/Ice/ServerTypes.cpp
		${CMAKE_CURRENT_BINARY_DIR}/Ice/Test.cpp
)

target_include_directories(icechecksumserver
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/Ice
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET icechecksumserver PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET icechecksumserver PROPERTY CXX_STANDARD 11)
endif()

if(BUILD_ICE_UWP)
	add_dependencies(icechecksumserver ${CHECKSUM_DEPS})
	set_property(TARGET icechecksumserver PROPERTY CXX_STANDARD 11)
else()
	target_link_libraries(icechecksumserver PRIVATE ${CHECKSUM_DEPS})
endif()

target_compile_definitions(icechecksumserver PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			ServerTypes.ice
			Test.ice
	)

	source_group("Source Files/Generated"
		FILES
			${CMAKE_CURRENT_BINARY_DIR}/Ice/ServerTypes.cpp
	)

	source_group("Header Files/Generated"
		FILES
			${CMAKE_CURRENT_BINARY_DIR}/Ice/ServerTypes.h
	)

	target_compile_options(icechecksumserver
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	if(NOT BUILD_ICE_UWP)
		set_property(TARGET icechecksumserver PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
	else()
		set_property(TARGET icechecksumserver PROPERTY LINK_FLAGS ${ICE_UWP_LINK_OPTIONS})
	endif()
endif()

install(TARGETS icechecksumserver DESTINATION tests/bin)
