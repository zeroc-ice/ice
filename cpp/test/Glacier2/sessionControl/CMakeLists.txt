if(NOT ${CMAKE_CURRENT_BINARY_DIR}/SessionControl)
	file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/SessionControl)
endif()

set(GENERATED_SESSIONCTRL_HEADERS ${CMAKE_CURRENT_BINARY_DIR}/SessionControl/Session.h)
set(GENERATED_SESSIONCTRL_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/SessionControl/Session.cpp)

if(BUILD_ICE_CPP98)
	set(SESSIONCTRL_DEPS 
		glacier2 
		ice
		icediscovery
		icelocatordiscovery
		icessl
		testcommon
	)
elseif(BUILD_ICE_CPP11)
	set(SESSIONCTRL_DEPS 
		glacier2++11 
		ice++11
		icediscovery++11
		icelocatordiscovery++11
		icessl++11
		testcommon++11
	)
endif()

list(APPEND ICE_SLICE_DIRS "-I${SLICE_DIR}")
list(APPEND ICE_SLICE_PARAMS ${COMPILE_SLICES_CPP_PARAMETERS} "SessionControl")

set(output_files 
	${CMAKE_CURRENT_BINARY_DIR}/SessionControl/Session.cpp 
	${CMAKE_CURRENT_BINARY_DIR}/SessionControl/Session.h
)	
#function(CompileSlice slice2bin_params slice_include_paths ice_file output_dir output_file)
CompileSlice("${ICE_SLICE_PARAMS}"
	"${ICE_SLICE_DIRS}"
	${CMAKE_CURRENT_SOURCE_DIR}/Session.ice
	${CMAKE_CURRENT_BINARY_DIR}/SessionControl
	"${output_files}"
)

add_executable(glacier2sessionctrlclient)

target_sources(glacier2sessionctrlclient 
	PRIVATE 
		Client.cpp
		${GENERATED_SESSIONCTRL_HEADERS}
		${GENERATED_SESSIONCTRL_SOURCES}
)

target_include_directories(glacier2sessionctrlclient
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/SessionControl
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET glacier2sessionctrlclient PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET glacier2sessionctrlclient PROPERTY CXX_STANDARD 11)
endif()

target_link_libraries(glacier2sessionctrlclient PRIVATE ${SESSIONCTRL_DEPS})

target_compile_definitions(glacier2sessionctrlclient PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Session.ice
	)

	source_group("Source Files/Generated"
		FILES
			${GENERATED_SESSIONCTRL_SOURCES}
	)

	source_group("Header Files/Generated"
		FILES
			${GENERATED_SESSIONCTRL_HEADERS}
	)

	target_compile_options(glacier2sessionctrlclient
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	set_property(TARGET glacier2sessionctrlclient PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
endif()

install(TARGETS glacier2sessionctrlclient DESTINATION tests/bin)

add_executable(glacier2sessionctrlserver)

target_sources(glacier2sessionctrlserver 
	PRIVATE 
		Server.cpp
		SessionI.cpp
		${GENERATED_SESSIONCTRL_HEADERS}
		${GENERATED_SESSIONCTRL_SOURCES}
)

target_include_directories(glacier2sessionctrlserver
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/SessionControl
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET glacier2sessionctrlserver PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET glacier2sessionctrlserver PROPERTY CXX_STANDARD 11)
endif()

target_link_libraries(glacier2sessionctrlserver PRIVATE ${SESSIONCTRL_DEPS})

target_compile_definitions(glacier2sessionctrlserver PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Session.ice
	)

	source_group("Source Files/Generated"
		FILES
			${GENERATED_SESSIONCTRL_SOURCES}
	)

	source_group("Header Files/Generated"
		FILES
			${GENERATED_SESSIONCTRL_HEADERS}
	)

	target_compile_options(glacier2sessionctrlserver
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	set_property(TARGET glacier2sessionctrlserver PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
endif()

install(TARGETS glacier2sessionctrlserver DESTINATION tools/bin)
