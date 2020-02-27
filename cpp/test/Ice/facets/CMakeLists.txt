if(NOT ${CMAKE_CURRENT_BINARY_DIR}/Ice)
	file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Ice)
endif()

set(GENERATED_FACETS_HEADERS ${CMAKE_CURRENT_BINARY_DIR}/Ice/Test.h)
set(GENERATED_FACETS_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/Ice/Test.cpp)

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
	set(FACETS_DEPS 
		ice
		testcommon
	)
elseif(BUILD_ICE_CPP11)
	set(FACETS_DEPS 
		ice++11
		testcommon++11
	)
elseif(BUILD_ICE_UWP)
	set(FACETS_DEPS 
		iceuwp++11
		testcommonuwp++11
	)
endif()

add_executable(icefacetsclient)

target_sources(icefacetsclient
	PRIVATE
		AllTests.cpp
		Client.cpp
		${GENERATED_FACETS_HEADERS}
		${GENERATED_FACETS_SOURCES}
)

target_include_directories(icefacetsclient
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/Ice
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET icefacetsclient PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET icefacetsclient PROPERTY CXX_STANDARD 11)
endif()

if(BUILD_ICE_UWP)
	add_dependencies(icefacetsclient ${FACETS_DEPS})
	set_property(TARGET icefacetsclient PROPERTY CXX_STANDARD 11)
else()
	target_link_libraries(icefacetsclient PRIVATE ${FACETS_DEPS})
endif()

target_compile_definitions(icefacetsclient PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Test.ice
	)

	source_group("Source Files/Generated"
		FILES
			${GENERATED_FACETS_SOURCES}
	)

	source_group("Header Files/Generated"
		FILES
			${GENERATED_FACETS_HEADERS}
	)

	target_compile_options(icefacetsclient
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	if(NOT BUILD_ICE_UWP)
		set_property(TARGET icefacetsclient PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
	else()
		set_property(TARGET icefacetsclient PROPERTY LINK_FLAGS ${ICE_UWP_LINK_OPTIONS})
	endif()
endif()

install(TARGETS icefacetsclient DESTINATION tests/bin)

add_executable(icefacetscollocated)

target_sources(icefacetscollocated
	PRIVATE
		AllTests.cpp
		Collocated.cpp
		TestI.cpp
		${GENERATED_FACETS_HEADERS}
		${GENERATED_FACETS_SOURCES}
)

target_include_directories(icefacetscollocated
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/Ice
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET icefacetscollocated PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET icefacetscollocated PROPERTY CXX_STANDARD 11)
endif()

if(BUILD_ICE_UWP)
	add_dependencies(icefacetscollocated ${FACETS_DEPS})
	set_property(TARGET icefacetscollocated PROPERTY CXX_STANDARD 11)
else()
	target_link_libraries(icefacetscollocated PRIVATE ${FACETS_DEPS})
endif()

target_compile_definitions(icefacetscollocated PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Test.ice
	)

	source_group("Source Files/Generated"
		FILES
			${GENERATED_FACETS_SOURCES}
	)

	source_group("Header Files/Generated"
		FILES
			${GENERATED_FACETS_HEADERS}
	)

	target_compile_options(icefacetscollocated
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	if(NOT BUILD_ICE_UWP)
		set_property(TARGET icefacetscollocated PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
	else()
		set_property(TARGET icefacetscollocated PROPERTY LINK_FLAGS ${ICE_UWP_LINK_OPTIONS})
	endif()
endif()

install(TARGETS icefacetscollocated DESTINATION tests/bin)

add_executable(icefacetsserver)

target_sources(icefacetsserver
	PRIVATE
		Server.cpp
		TestI.cpp
		${GENERATED_FACETS_HEADERS}
		${GENERATED_FACETS_SOURCES}
)

target_include_directories(icefacetsserver
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/Ice
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET icefacetsserver PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET icefacetsserver PROPERTY CXX_STANDARD 11)
endif()

if(BUILD_ICE_UWP)
	add_dependencies(icefacetsserver ${FACETS_DEPS})
	set_property(TARGET icefacetsserver PROPERTY CXX_STANDARD 11)
else()
	target_link_libraries(icefacetsserver PRIVATE ${FACETS_DEPS})
endif()

target_compile_definitions(icefacetsserver PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Test.ice
	)

	source_group("Source Files/Generated"
		FILES
			${GENERATED_FACETS_SOURCES}
	)

	source_group("Header Files/Generated"
		FILES
			${GENERATED_FACETS_HEADERS}
	)

	target_compile_options(icefacetsserver
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	if(NOT BUILD_ICE_UWP)
		set_property(TARGET icefacetsserver PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
	else()
		set_property(TARGET icefacetsserver PROPERTY LINK_FLAGS ${ICE_UWP_LINK_OPTIONS})
	endif()
endif()

install(TARGETS icefacetsserver DESTINATION tests/bin)
