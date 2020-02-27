if(NOT ${CMAKE_CURRENT_BINARY_DIR}/Ice)
	file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Ice)
endif()

set(GENERATED_INHERITANCE_HEADERS ${CMAKE_CURRENT_BINARY_DIR}/Ice/Test.h)
set(GENERATED_INHERITANCE_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/Ice/Test.cpp)

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
	set(INHERITANCE_DEPS 
		ice
		testcommon
	)
elseif(BUILD_ICE_CPP11)
	set(INHERITANCE_DEPS 
		ice++11
		testcommon++11
	)
elseif(BUILD_ICE_UWP)
	set(INHERITANCE_DEPS 
		iceuwp++11
		testcommonuwp++11
	)
endif()

add_executable(iceinheritanceclient)

target_sources(iceinheritanceclient
	PRIVATE
		AllTests.cpp
		Client.cpp
		${GENERATED_INHERITANCE_HEADERS}
		${GENERATED_INHERITANCE_SOURCES}
)

target_include_directories(iceinheritanceclient
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/Ice
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET iceinheritanceclient PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET iceinheritanceclient PROPERTY CXX_STANDARD 11)
endif()

if(BUILD_ICE_UWP)
	add_dependencies(iceinheritanceclient ${INHERITANCE_DEPS})
	set_property(TARGET iceinheritanceclient PROPERTY CXX_STANDARD 11)
else()
	target_link_libraries(iceinheritanceclient PRIVATE ${INHERITANCE_DEPS})
endif()

target_compile_definitions(iceinheritanceclient PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Test.ice
	)

	source_group("Source Files/Generated"
		FILES
			${GENERATED_INHERITANCE_SOURCES}
	)

	source_group("Header Files/Generated"
		FILES
			${GENERATED_INHERITANCE_HEADERS}
	)

	target_compile_options(iceinheritanceclient
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	if(NOT BUILD_ICE_UWP)
		set_property(TARGET iceinheritanceclient PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
	else()
		set_property(TARGET iceinheritanceclient PROPERTY LINK_FLAGS ${ICE_UWP_LINK_OPTIONS})
	endif()
endif()

install(TARGETS iceinheritanceclient DESTINATION tests/bin)

add_executable(iceinheritancecollocated)

target_sources(iceinheritancecollocated
	PRIVATE
		AllTests.cpp
		Collocated.cpp
		TestI.cpp
		${GENERATED_INHERITANCE_HEADERS}
		${GENERATED_INHERITANCE_SOURCES}
)

target_include_directories(iceinheritancecollocated
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/Ice
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET iceinheritancecollocated PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET iceinheritancecollocated PROPERTY CXX_STANDARD 11)
endif()

if(BUILD_ICE_UWP)
	add_dependencies(iceinheritancecollocated ${INHERITANCE_DEPS})
	set_property(TARGET iceinheritancecollocated PROPERTY CXX_STANDARD 11)
else()
	target_link_libraries(iceinheritancecollocated PRIVATE ${INHERITANCE_DEPS})
endif()

target_compile_definitions(iceinheritancecollocated PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Test.ice
	)

	source_group("Source Files/Generated"
		FILES
			${GENERATED_INHERITANCE_SOURCES}
	)

	source_group("Header Files/Generated"
		FILES
			${GENERATED_INHERITANCE_HEADERS}
	)

	target_compile_options(iceinheritancecollocated
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	if(NOT BUILD_ICE_UWP)
		set_property(TARGET iceinheritancecollocated PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
	else()
		set_property(TARGET iceinheritancecollocated PROPERTY LINK_FLAGS ${ICE_UWP_LINK_OPTIONS})
	endif()
endif()

install(TARGETS iceinheritancecollocated DESTINATION tests/bin)

add_executable(iceinheritanceserver)

target_sources(iceinheritanceserver
	PRIVATE
		Server.cpp
		TestI.cpp
		${GENERATED_INHERITANCE_HEADERS}
		${GENERATED_INHERITANCE_SOURCES}
)

target_include_directories(iceinheritanceserver
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/Ice
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET iceinheritanceserver PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET iceinheritanceserver PROPERTY CXX_STANDARD 11)
endif()

if(BUILD_ICE_UWP)
	add_dependencies(iceinheritanceserver ${INHERITANCE_DEPS})
	set_property(TARGET iceinheritanceserver PROPERTY CXX_STANDARD 11)
else()
	target_link_libraries(iceinheritanceserver PRIVATE ${INHERITANCE_DEPS})
endif()

target_compile_definitions(iceinheritanceserver PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Test.ice
	)

	source_group("Source Files/Generated"
		FILES
			${GENERATED_INHERITANCE_SOURCES}
	)

	source_group("Header Files/Generated"
		FILES
			${GENERATED_INHERITANCE_HEADERS}
	)

	target_compile_options(iceinheritanceserver
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	if(NOT BUILD_ICE_UWP)
		set_property(TARGET iceinheritanceserver PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
	else()
		set_property(TARGET iceinheritanceserver PROPERTY LINK_FLAGS ${ICE_UWP_LINK_OPTIONS})
	endif()
endif()

install(TARGETS iceinheritanceserver DESTINATION tests/bin)
