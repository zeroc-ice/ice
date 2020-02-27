if(NOT ${CMAKE_CURRENT_BINARY_DIR}/IceBox)
	file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/IceBox)
endif()

set(GENERATED_ADMIN_HEADERS ${CMAKE_CURRENT_BINARY_DIR}/IceBox/Test.h)
set(GENERATED_ADMIN_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/IceBox/Test.cpp)

list(APPEND ICE_SLICE_DIRS "-I${SLICE_DIR}")
list(APPEND ICE_SLICE_PARAMS ${COMPILE_SLICES_CPP_PARAMETERS} "IceBox")

set(output_files 
	${CMAKE_CURRENT_BINARY_DIR}/IceBox/Test.cpp 
	${CMAKE_CURRENT_BINARY_DIR}/IceBox/Test.h
)	
#function(CompileSlice slice2bin_params slice_include_paths ice_file output_dir output_file)
CompileSlice("${ICE_SLICE_PARAMS}"
	"${ICE_SLICE_DIRS}"
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	${CMAKE_CURRENT_BINARY_DIR}/IceBox
	"${output_files}"
)

if(BUILD_ICE_CPP98)
	set(CLIENT_DEPS 
		ice
		testcommon
	)

	set(SERVICE_DEPS
		ice
		iceboxlib
		testcommon
	)
elseif(BUILD_ICE_CPP11)
	set(CLIENT_DEPS 
		ice++11
		testcommon++11
	)

	set(SERVICE_DEPS
		ice++11
		iceboxlib++11
		testcommon++11
	)
endif()

add_executable(iceboxadminclient)

target_sources(iceboxadminclient
	PRIVATE
		AllTests.cpp
		Client.cpp
		${GENERATED_ADMIN_HEADERS}
		${GENERATED_ADMIN_SOURCES}
)

target_include_directories(iceboxadminclient
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/IceBox
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET iceboxadminclient PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET iceboxadminclient PROPERTY CXX_STANDARD 11)
endif()

target_link_libraries(iceboxadminclient PRIVATE ${CLIENT_DEPS})

target_compile_definitions(iceboxadminclient PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Test.ice
	)

	source_group("Source Files/Generated"
		FILES
			${GENERATED_ADMIN_SOURCES}
	)

	source_group("Header Files/Generated"
		FILES
			${GENERATED_ADMIN_HEADERS}
	)

	target_compile_options(iceboxadminclient
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	set_property(TARGET iceboxadminclient PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
endif()

install(TARGETS iceboxadminclient DESTINATION tests/bin)

add_library(iceboxadmintestservice)

target_sources(iceboxadmintestservice
	PRIVATE
		Service.cpp
		TestI.cpp
		${GENERATED_ADMIN_HEADERS}
		${GENERATED_ADMIN_SOURCES}
)

target_include_directories(iceboxadmintestservice
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/IceBox
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_target_properties(iceboxadmintestservice 
		PROPERTIES 
			CXX_STANDARD 98
			OUTPUT_NAME testservice
	)
elseif(BUILD_ICE_CPP11)
	set_target_properties(iceboxadmintestservice 
		PROPERTY 
			CXX_STANDARD 11
			OUTPUT_NAME testservice++11
	)
endif()

target_link_libraries(iceboxadmintestservice PRIVATE ${SERVICE_DEPS})

target_compile_definitions(iceboxadmintestservice PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Test.ice
	)

	source_group("Source Files/Generated"
		FILES
			${GENERATED_ADMIN_SOURCES}
	)

	source_group("Header Files/Generated"
		FILES
			${GENERATED_ADMIN_HEADERS}
	)

	target_compile_options(iceboxadmintestservice
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	set_property(TARGET iceboxadmintestservice PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
endif()

if(BUILD_SHARED_LIBS)
	install(TARGETS iceboxadmintestservice DESTINATION tests/bin)
else()
	install(TARGETS iceboxadmintestservice DESTINATION tests/lib)
endif()

