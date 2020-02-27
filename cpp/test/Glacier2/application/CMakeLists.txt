if(NOT ${CMAKE_CURRENT_BINARY_DIR}/Glacier2)
	file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Glacier2)
endif()

set(GENERATED_APPLICATION_HEADERS ${CMAKE_CURRENT_BINARY_DIR}/Glacier2/Callback.h)
set(GENERATED_APPLICATION_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/Glacier2/Callback.cpp)

if(BUILD_ICE_CPP98)
	set(APPLICATION_DEPS 
		glacier2 
		ice
		icediscovery
		icelocatordiscovery
		icessl
		testcommon
	)
elseif(BUILD_ICE_CPP11)
	set(APPLICATION_DEPS 
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

add_executable(glacier2appclient)

target_sources(glacier2appclient 
	PRIVATE 
		Client.cpp
		${GENERATED_APPLICATION_HEADERS}
		${GENERATED_APPLICATION_SOURCES}
)

target_include_directories(glacier2appclient
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_BINARY_DIR}/Glacier2
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET glacier2appclient PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET glacier2appclient PROPERTY CXX_STANDARD 11)
endif()

target_link_libraries(glacier2appclient PRIVATE ${APPLICATION_DEPS})

target_compile_definitions(glacier2appclient PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Callback.ice
	)

	source_group("Source Files/Generated"
		FILES
			${GENERATED_APPLICATION_SOURCES}
	)

	source_group("Header Files/Generated"
		FILES
			${GENERATED_APPLICATION_HEADERS}
	)

	target_compile_options(glacier2appclient
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	set_property(TARGET glacier2appclient PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
endif()

install(TARGETS glacier2appclient DESTINATION tests/bin)

add_executable(glacier2appserver)

target_sources(glacier2appserver 
	PRIVATE 
		Server.cpp
		${GENERATED_APPLICATION_HEADERS}
		${GENERATED_APPLICATION_SOURCES}
)

target_include_directories(glacier2appserver
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_BINARY_DIR}/Glacier2
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET glacier2appserver PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET glacier2appserver PROPERTY CXX_STANDARD 11)
endif()

target_link_libraries(glacier2appserver PRIVATE ${APPLICATION_DEPS})

target_compile_definitions(glacier2appserver PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Callback.ice
	)

	source_group("Source Files/Generated"
		FILES
			${GENERATED_APPLICATION_SOURCES}
	)

	source_group("Header Files/Generated"
		FILES
			${GENERATED_APPLICATION_HEADERS}
	)

	target_compile_options(glacier2appserver
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	set_property(TARGET glacier2appserver PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
endif()

install(TARGETS glacier2appserver DESTINATION tools/bin)
