if(NOT ${CMAKE_CURRENT_BINARY_DIR}/Glacier2)
	file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Glacier2)
endif()

set(GENERATED_SESSIONHLPR_HEADERS ${CMAKE_CURRENT_BINARY_DIR}/Glacier2/Callback.h)
set(GENERATED_SESSIONHLPR_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/Glacier2/Callback.cpp)

if(BUILD_ICE_CPP98)
	set(SESSIONHLPR_DEPS 
		glacier2 
		ice
		icediscovery
		icelocatordiscovery
		icessl
		testcommon
	)
elseif(BUILD_ICE_CPP11)
	set(SESSIONHLPR_DEPS 
		glacier2++11 
		ice++11
		icediscovery++11
		icelocatordiscovery++11
		icessl++11
		testcommon++11
	)
endif()

list(APPEND ICE_SLICE_DIRS "-I${SLICE_DIR}")
list(APPEND ICE_SLICE_PARAMS ${COMPILE_SLICES_CPP_PARAMETERS} "Glacier2")

set(output_files 
	${CMAKE_CURRENT_BINARY_DIR}/Glacier2/Callback.cpp 
	${CMAKE_CURRENT_BINARY_DIR}/Glacier2/Callback.h
)	
#function(CompileSlice slice2bin_params slice_include_paths ice_file output_dir output_file)
CompileSlice("${ICE_SLICE_PARAMS}"
	"${ICE_SLICE_DIRS}"
	${CMAKE_CURRENT_SOURCE_DIR}/Callback.ice
	${CMAKE_CURRENT_BINARY_DIR}/Glacier2
	"${output_files}"
)

add_executable(glacier2sessionhlprclient)

target_sources(glacier2sessionhlprclient 
	PRIVATE 
		Client.cpp
		${GENERATED_SESSIONHLPR_HEADERS}
		${GENERATED_SESSIONHLPR_SOURCES}
)

target_include_directories(glacier2sessionhlprclient
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/Glacier2
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET glacier2sessionhlprclient PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET glacier2sessionhlprclient PROPERTY CXX_STANDARD 11)
endif()

target_link_libraries(glacier2sessionhlprclient PRIVATE ${SESSIONHLPR_DEPS})

target_compile_definitions(glacier2sessionhlprclient PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Callback.ice
	)

	source_group("Source Files/Generated"
		FILES
			${GENERATED_SESSIONHLPR_SOURCES}
	)

	source_group("Header Files/Generated"
		FILES
			${GENERATED_SESSIONHLPR_HEADERS}
	)

	target_compile_options(glacier2sessionhlprclient
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	if(NOT BUILD_ICE_UWP)
		set_property(TARGET glacier2sessionhlprclient PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
	else()
		set_property(TARGET glacier2sessionhlprclient PROPERTY LINK_FLAGS ${ICE_UWP_LINK_OPTIONS})
	endif()
endif()

install(TARGETS glacier2sessionhlprclient DESTINATION tests/bin)

add_executable(glacier2sessionhlprserver)

target_sources(glacier2sessionhlprserver 
	PRIVATE 
		Server.cpp
		${GENERATED_SESSIONHLPR_HEADERS}
		${GENERATED_SESSIONHLPR_SOURCES}
)

target_include_directories(glacier2sessionhlprserver
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/Glacier2
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET glacier2sessionhlprserver PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET glacier2sessionhlprserver PROPERTY CXX_STANDARD 11)
endif()

target_link_libraries(glacier2sessionhlprserver PRIVATE ${SESSIONHLPR_DEPS})

target_compile_definitions(glacier2sessionhlprserver PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Callback.ice
	)

	source_group("Source Files/Generated"
		FILES
			${GENERATED_SESSIONHLPR_SOURCES}
	)

	source_group("Header Files/Generated"
		FILES
			${GENERATED_SESSIONHLPR_HEADERS}
	)

	target_compile_options(glacier2sessionhlprserver
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	set_property(TARGET glacier2sessionhlprserver PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
endif()

install(TARGETS glacier2sessionhlprserver DESTINATION tools/bin)
