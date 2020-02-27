if(NOT ${CMAKE_CURRENT_BINARY_DIR}/Ice)
	file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Ice)
endif()

set(SLICING_EXCEPTIONS_SLICES
	${CMAKE_CURRENT_SOURCE_DIR}/ClientPrivate.ice
	${CMAKE_CURRENT_SOURCE_DIR}/ServerPrivate.ice
	${CMAKE_CURRENT_SOURCE_DIR}/ServerPrivateAMD.ice
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	${CMAKE_CURRENT_SOURCE_DIR}/TestAMD.ice
)

list(APPEND ICE_SLICE_DIRS "-I${CMAKE_CURRENT_SOURCE_DIR}")
list(APPEND ICE_SLICE_PARAMS ${COMPILE_SLICES_CPP_PARAMETERS} "Ice")

foreach(slice ${SLICING_EXCEPTIONS_SLICES})
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
	set(SLICING_EXCEPTIONS_DEPS 
		ice
		testcommon
	)
elseif(BUILD_ICE_CPP11)
	set(SLICING_EXCEPTIONS_DEPS 
		ice++11
		testcommon++11
	)
elseif(BUILD_ICE_UWP)
	set(SLICING_EXCEPTIONS_DEPS 
		iceuwp++11
		testcommonuwp++11
	)
endif()

add_executable(iceslicingexclient)

target_sources(iceslicingexclient
	PRIVATE
		AllTests.cpp
		Client.cpp
		${CMAKE_CURRENT_BINARY_DIR}/Ice/Test.cpp
		${CMAKE_CURRENT_BINARY_DIR}/Ice/ClientPrivate.cpp
)

target_include_directories(iceslicingexclient
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/Ice
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET iceslicingexclient PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET iceslicingexclient PROPERTY CXX_STANDARD 11)
endif()

if(BUILD_ICE_UWP)
	add_dependencies(iceslicingexclient ${SLICING_EXCEPTIONS_DEPS})
	set_property(TARGET iceslicingexclient PROPERTY CXX_STANDARD 11)
else()
	target_link_libraries(iceslicingexclient PRIVATE ${SLICING_EXCEPTIONS_DEPS})
endif()

target_compile_definitions(iceslicingexclient PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Test.ice
			ClientPrivate.ice
	)

	source_group("Source Files/Generated"
		FILES
			${CMAKE_CURRENT_BINARY_DIR}/Ice/Test.cpp
			${CMAKE_CURRENT_BINARY_DIR}/Ice/ClientPrivate.cpp
	)

	source_group("Header Files/Generated"
		FILES
			${CMAKE_CURRENT_BINARY_DIR}/Ice/Test.h
			${CMAKE_CURRENT_BINARY_DIR}/Ice/ClientPrivate.h
	)

	target_compile_options(iceslicingexclient
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	if(NOT BUILD_ICE_UWP)
		set_property(TARGET iceslicingexclient PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
	else()
		set_property(TARGET iceslicingexclient PROPERTY LINK_FLAGS ${ICE_UWP_LINK_OPTIONS})
	endif()
endif()

install(TARGETS iceslicingexclient DESTINATION tests/bin)

add_executable(iceslicingexserver)

target_sources(iceslicingexserver
	PRIVATE
		Server.cpp
		TestI.cpp
		${CMAKE_CURRENT_BINARY_DIR}/Ice/Test.cpp
		${CMAKE_CURRENT_BINARY_DIR}/Ice/ServerPrivate.cpp
)

target_include_directories(iceslicingexserver
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/Ice
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET iceslicingexserver PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET iceslicingexserver PROPERTY CXX_STANDARD 11)
endif()

if(BUILD_ICE_UWP)
	add_dependencies(iceslicingexserver ${SLICING_EXCEPTIONS_DEPS})
	set_property(TARGET iceslicingexserver PROPERTY CXX_STANDARD 11)
else()
	target_link_libraries(iceslicingexserver PRIVATE ${SLICING_EXCEPTIONS_DEPS})
endif()

target_compile_definitions(iceslicingexserver PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Test.ice
			ServerPrivate.ice
	)

	source_group("Source Files/Generated"
		FILES
			${CMAKE_CURRENT_BINARY_DIR}/Ice/Test.cpp
			${CMAKE_CURRENT_BINARY_DIR}/Ice/ServerPrivate.cpp
	)

	source_group("Header Files/Generated"
		FILES
			${CMAKE_CURRENT_BINARY_DIR}/Ice/Test.h
			${CMAKE_CURRENT_BINARY_DIR}/Ice/ServerPrivate.h
	)

	target_compile_options(iceslicingexserver
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	if(NOT BUILD_ICE_UWP)
		set_property(TARGET iceslicingexserver PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
	else()
		set_property(TARGET iceslicingexserver PROPERTY LINK_FLAGS ${ICE_UWP_LINK_OPTIONS})
	endif()
endif()

install(TARGETS iceslicingexserver DESTINATION tests/bin)

add_executable(iceslicingexserveramd)

target_sources(iceslicingexserveramd
	PRIVATE
		ServerAMD.cpp
		TestAMDI.cpp
		${CMAKE_CURRENT_BINARY_DIR}/Ice/TestAMD.cpp
		${CMAKE_CURRENT_BINARY_DIR}/Ice/ServerPrivateAMD.cpp
)

target_include_directories(iceslicingexserveramd
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/Ice
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET iceslicingexserveramd PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET iceslicingexserveramd PROPERTY CXX_STANDARD 11)
endif()

if(BUILD_ICE_UWP)
	add_dependencies(iceslicingexserveramd ${SLICING_EXCEPTIONS_DEPS})
	set_property(TARGET iceslicingexserveramd PROPERTY CXX_STANDARD 11)
else()
	target_link_libraries(iceslicingexserveramd PRIVATE ${SLICING_EXCEPTIONS_DEPS})
endif()

target_compile_definitions(iceslicingexserveramd PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Test.ice
			ServerPrivateAMD.ice
	)

	source_group("Source Files/Generated"
		FILES
			${CMAKE_CURRENT_BINARY_DIR}/Ice/TestAMD.cpp
			${CMAKE_CURRENT_BINARY_DIR}/Ice/ServerPrivateAMD.cpp
	)

	source_group("Header Files/Generated"
		FILES
			${CMAKE_CURRENT_BINARY_DIR}/Ice/TestAMD.h
			${CMAKE_CURRENT_BINARY_DIR}/Ice/ServerPrivateAMD.h
	)

	target_compile_options(iceslicingexserveramd
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	if(NOT BUILD_ICE_UWP)
		set_property(TARGET iceslicingexserveramd PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
	else()
		set_property(TARGET iceslicingexserveramd PROPERTY LINK_FLAGS ${ICE_UWP_LINK_OPTIONS})
	endif()
endif()

install(TARGETS iceslicingexserveramd DESTINATION tests/bin)
