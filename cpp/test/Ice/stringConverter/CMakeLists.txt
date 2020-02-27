if(NOT ${CMAKE_CURRENT_BINARY_DIR}/Ice)
	file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Ice)
endif()

set(GENERATED_STRINGCONVERTER_HEADERS ${CMAKE_CURRENT_BINARY_DIR}/Ice/Test.h)
set(GENERATED_STRINGCONVERTER_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/Ice/Test.cpp)

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
	set(STRINGCONVERTER_DEPS 
		ice
		testcommon
	)
elseif(BUILD_ICE_CPP11)
	set(STRINGCONVERTER_DEPS 
		ice++11
		testcommon++11
	)
elseif(BUILD_ICE_UWP)
	set(STRINGCONVERTER_DEPS 
		iceuwp++11
		testcommonuwp++11
	)
endif()

add_executable(icestringconverterclient)

target_sources(icestringconverterclient
	PRIVATE
		Client.cpp
		${GENERATED_STRINGCONVERTER_HEADERS}
		${GENERATED_STRINGCONVERTER_SOURCES}
)

target_include_directories(icestringconverterclient
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/Ice
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET icestringconverterclient PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET icestringconverterclient PROPERTY CXX_STANDARD 11)
endif()

if(BUILD_ICE_UWP)
	add_dependencies(icestringconverterclient ${STRINGCONVERTER_DEPS})
	set_property(TARGET icestringconverterclient PROPERTY CXX_STANDARD 11)
else()
	target_link_libraries(icestringconverterclient PRIVATE ${STRINGCONVERTER_DEPS})
endif()

target_compile_definitions(icestringconverterclient PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Test.ice
	)

	source_group("Source Files/Generated"
		FILES
			${GENERATED_STRINGCONVERTER_SOURCES}
	)

	source_group("Header Files/Generated"
		FILES
			${GENERATED_STRINGCONVERTER_HEADERS}
	)

	target_compile_options(icestringconverterclient
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	if(NOT BUILD_ICE_UWP)
		set_property(TARGET icestringconverterclient PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
	else()
		set_property(TARGET icestringconverterclient PROPERTY LINK_FLAGS ${ICE_UWP_LINK_OPTIONS})
	endif()
endif()

install(TARGETS icestringconverterclient DESTINATION tests/bin)

add_executable(icestringconverterserver)

target_sources(icestringconverterserver
	PRIVATE
		Server.cpp
		${GENERATED_STRINGCONVERTER_HEADERS}
		${GENERATED_STRINGCONVERTER_SOURCES}
)

target_include_directories(icestringconverterserver
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/Ice
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET icestringconverterserver PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET icestringconverterserver PROPERTY CXX_STANDARD 11)
endif()

if(BUILD_ICE_UWP)
	add_dependencies(icestringconverterserver ${STRINGCONVERTER_DEPS})
	set_property(TARGET icestringconverterserver PROPERTY CXX_STANDARD 11)
else()
	target_link_libraries(icestringconverterserver PRIVATE ${STRINGCONVERTER_DEPS})
endif()

target_compile_definitions(icestringconverterserver PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Test.ice
	)

	source_group("Source Files/Generated"
		FILES
			${GENERATED_STRINGCONVERTER_SOURCES}
	)

	source_group("Header Files/Generated"
		FILES
			${GENERATED_STRINGCONVERTER_HEADERS}
	)

	target_compile_options(icestringconverterserver
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	if(NOT BUILD_ICE_UWP)
		set_property(TARGET icestringconverterserver PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
	else()
		set_property(TARGET icestringconverterserver PROPERTY LINK_FLAGS ${ICE_UWP_LINK_OPTIONS})
	endif()
endif()

install(TARGETS icestringconverterserver DESTINATION tests/bin)
