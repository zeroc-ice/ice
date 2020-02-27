if(NOT ${CMAKE_CURRENT_BINARY_DIR}/Ice)
	file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Ice)
endif()

set(CUSTOM_SLICES
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	${CMAKE_CURRENT_SOURCE_DIR}/TestAMD.ice
	${CMAKE_CURRENT_SOURCE_DIR}/Wstring.ice
	${CMAKE_CURRENT_SOURCE_DIR}/WstringAMD.ice
)

list(APPEND ICE_SLICE_DIRS "-I${SLICE_DIR}")
list(APPEND ICE_SLICE_PARAMS ${COMPILE_SLICES_CPP_PARAMETERS} "Ice")

foreach(slice ${CUSTOM_SLICES})
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
	set(CUSTOM_DEPS 
		ice
		testcommon
	)
elseif(BUILD_ICE_CPP11)
	set(CUSTOM_DEPS 
		ice++11
		testcommon++11
	)
elseif(BUILD_ICE_UWP)
	set(CUSTOM_DEPS 
		iceuwp++11
		testcommonuwp++11
	)
endif()

add_executable(icecustomclient)

target_sources(icecustomclient
	PRIVATE
		AllTests.cpp
		Client.cpp
		MyByteSeq.cpp
		StringConverterI.cpp
		${CMAKE_CURRENT_BINARY_DIR}/Ice/Test.cpp
		${CMAKE_CURRENT_BINARY_DIR}/Ice/Wstring.cpp
)

target_include_directories(icecustomclient
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/Ice
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET icecustomclient PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET icecustomclient PROPERTY CXX_STANDARD 11)
endif()

if(BUILD_ICE_UWP)
	add_dependencies(icecustomclient ${CUSTOM_DEPS})
	set_property(TARGET icecustomclient PROPERTY CXX_STANDARD 11)
else()
	target_link_libraries(icecustomclient PRIVATE ${CUSTOM_DEPS})
endif()

target_compile_definitions(icecustomclient PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Test.ice
			Wstring.ice
	)

	source_group("Source Files/Generated"
		FILES
			${CMAKE_CURRENT_BINARY_DIR}/Ice/Test.cpp
			${CMAKE_CURRENT_BINARY_DIR}/Ice/Wstring.cpp
	)

	source_group("Header Files/Generated"
		FILES
			${CMAKE_CURRENT_BINARY_DIR}/Ice/Test.h
			${CMAKE_CURRENT_BINARY_DIR}/Ice/Wstring.h
	)

	target_compile_options(icecustomclient
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	if(NOT BUILD_ICE_UWP)
		set_property(TARGET icecustomclient PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
	else()
		set_property(TARGET icecustomclient PROPERTY LINK_FLAGS ${ICE_UWP_LINK_OPTIONS})
	endif()
endif()

install(TARGETS icecustomclient DESTINATION tests/bin)

add_executable(icecustomcollocated)

target_sources(icecustomcollocated
	PRIVATE
		AllTests.cpp
		Collocated.cpp
		MyByteSeq.cpp
		StringConverterI.cpp
		TestI.cpp
		WstringI.cpp
		${CMAKE_CURRENT_BINARY_DIR}/Ice/Test.cpp
		${CMAKE_CURRENT_BINARY_DIR}/Ice/Wstring.cpp
)

target_include_directories(icecustomcollocated
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/Ice
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET icecustomcollocated PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET icecustomcollocated PROPERTY CXX_STANDARD 11)
endif()

if(BUILD_ICE_UWP)
	add_dependencies(icecustomcollocated ${CUSTOM_DEPS})
	set_property(TARGET icecustomcollocated PROPERTY CXX_STANDARD 11)
else()
	target_link_libraries(icecustomcollocated PRIVATE ${CUSTOM_DEPS})
endif()

target_compile_definitions(icecustomcollocated PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Test.ice
			Wstring.ice
	)

	source_group("Source Files/Generated"
		FILES
			${CMAKE_CURRENT_BINARY_DIR}/Ice/Test.cpp
			${CMAKE_CURRENT_BINARY_DIR}/Ice/Wstring.cpp
	)

	source_group("Header Files/Generated"
		FILES
			${CMAKE_CURRENT_BINARY_DIR}/Ice/Test.h
			${CMAKE_CURRENT_BINARY_DIR}/Ice/Wstring.h
	)

	target_compile_options(icecustomcollocated
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	if(NOT BUILD_ICE_UWP)
		set_property(TARGET icecustomcollocated PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
	else()
		set_property(TARGET icecustomcollocated PROPERTY LINK_FLAGS ${ICE_UWP_LINK_OPTIONS})
	endif()
endif()

install(TARGETS icecustomcollocated DESTINATION tests/bin)

add_executable(icecustomserver)

target_sources(icecustomserver
	PRIVATE
		MyByteSeq.cpp
		Server.cpp
		StringConverterI.cpp
		TestI.cpp
		WstringI.cpp
		${CMAKE_CURRENT_BINARY_DIR}/Ice/Test.cpp
		${CMAKE_CURRENT_BINARY_DIR}/Ice/Wstring.cpp
)

target_include_directories(icecustomserver
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/Ice
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET icecustomserver PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET icecustomserver PROPERTY CXX_STANDARD 11)
endif()

if(BUILD_ICE_UWP)
	add_dependencies(icecustomserver ${CUSTOM_DEPS})
	set_property(TARGET icecustomserver PROPERTY CXX_STANDARD 11)
else()
	target_link_libraries(icecustomserver PRIVATE ${CUSTOM_DEPS})
endif()

target_compile_definitions(icecustomserver PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Test.ice
			Wstring.ice
	)

	source_group("Source Files/Generated"
		FILES
			${CMAKE_CURRENT_BINARY_DIR}/Ice/Test.cpp
			${CMAKE_CURRENT_BINARY_DIR}/Ice/Wstring.cpp
	)

	source_group("Header Files/Generated"
		FILES
			${CMAKE_CURRENT_BINARY_DIR}/Ice/Test.h
			${CMAKE_CURRENT_BINARY_DIR}/Ice/Wstring.h
	)

	target_compile_options(icecustomserver
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	if(NOT BUILD_ICE_UWP)
		set_property(TARGET icecustomserver PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
	else()
		set_property(TARGET icecustomserver PROPERTY LINK_FLAGS ${ICE_UWP_LINK_OPTIONS})
	endif()
endif()

install(TARGETS icecustomserver DESTINATION tests/bin)

add_executable(icecustomserveramd)

target_sources(icecustomserveramd
	PRIVATE
		MyByteSeq.cpp
		ServerAMD.cpp
		StringConverterI.cpp
		TestAMDI.cpp
		WstringAMDI.cpp
		${CMAKE_CURRENT_BINARY_DIR}/Ice/TestAMD.cpp
		${CMAKE_CURRENT_BINARY_DIR}/Ice/WstringAMD.cpp
)

target_include_directories(icecustomserveramd
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/Ice
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET icecustomserveramd PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET icecustomserveramd PROPERTY CXX_STANDARD 11)
endif()

if(BUILD_ICE_UWP)
	add_dependencies(icecustomserveramd ${CUSTOM_DEPS})
	set_property(TARGET icecustomserveramd PROPERTY CXX_STANDARD 11)
else()
	target_link_libraries(icecustomserveramd PRIVATE ${CUSTOM_DEPS})
endif()

target_compile_definitions(icecustomserveramd PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Test.ice
			WstringAMD.ice
	)

	source_group("Source Files/Generated"
		FILES
			${CMAKE_CURRENT_BINARY_DIR}/Ice/TestAMD.cpp
			${CMAKE_CURRENT_BINARY_DIR}/Ice/WstringAMD.cpp
	)

	source_group("Header Files/Generated"
		FILES
			${CMAKE_CURRENT_BINARY_DIR}/Ice/TestAMD.h
			${CMAKE_CURRENT_BINARY_DIR}/Ice/WstringAMD.h
	)

	target_compile_options(icecustomserveramd
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	if(NOT BUILD_ICE_UWP)
		set_property(TARGET icecustomserveramd PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
	else()
		set_property(TARGET icecustomserveramd PROPERTY LINK_FLAGS ${ICE_UWP_LINK_OPTIONS})
	endif()
endif()

install(TARGETS icecustomserveramd DESTINATION tests/bin)
