if(NOT ${CMAKE_CURRENT_BINARY_DIR}/Objects)
	file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Objects)
endif()

set(OBJECTS_SLICES
	${CMAKE_CURRENT_SOURCE_DIR}/Forward.ice
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	${CMAKE_CURRENT_SOURCE_DIR}/Derived.ice
	${CMAKE_CURRENT_SOURCE_DIR}/DerivedEx.ice
)

set(GENERATED_OBJECTS_HEADERS 
	${CMAKE_CURRENT_BINARY_DIR}/Objects/Derived.h
	${CMAKE_CURRENT_BINARY_DIR}/Objects/DerivedEx.h
	${CMAKE_CURRENT_BINARY_DIR}/Objects/Forward.h
	${CMAKE_CURRENT_BINARY_DIR}/Objects/Test.h
)

set(GENERATED_OBJECTS_SOURCES 
	${CMAKE_CURRENT_BINARY_DIR}/Objects/Derived.cpp
	${CMAKE_CURRENT_BINARY_DIR}/Objects/DerivedEx.cpp
	${CMAKE_CURRENT_BINARY_DIR}/Objects/Forward.h
	${CMAKE_CURRENT_BINARY_DIR}/Objects/Test.cpp
)

list(APPEND ICE_SLICE_DIRS "-I${CMAKE_CURRENT_SOURCE_DIR}")
list(APPEND ICE_SLICE_PARAMS ${COMPILE_SLICES_CPP_PARAMETERS} "Objects")

foreach(slice ${OBJECTS_SLICES})
	get_filename_component(slice_name ${slice} NAME_WE)
	set(output_files 
		${CMAKE_CURRENT_BINARY_DIR}/Objects/${slice_name}.cpp 
		${CMAKE_CURRENT_BINARY_DIR}/Objects/${slice_name}.h
	)	
	#function(CompileSlice slice2bin_params slice_include_paths ice_file output_dir output_file)
	CompileSlice("${ICE_SLICE_PARAMS}"
		"${ICE_SLICE_DIRS}"
		${slice}
		${CMAKE_CURRENT_BINARY_DIR}/Objects
		"${output_files}"
	)
endforeach()

if(BUILD_ICE_CPP98)
	set(OBJECTS_DEPS 
		ice
		testcommon
	)
elseif(BUILD_ICE_CPP11)
	set(OBJECTS_DEPS 
		ice++11
		testcommon++11
	)
elseif(BUILD_ICE_UWP)
	set(OBJECTS_DEPS 
		iceuwp++11
		testcommonuwp++11
	)
endif()

add_executable(iceobjectsclient)

target_sources(iceobjectsclient
	PRIVATE
		AllTests.cpp
		Client.cpp
		TestI.cpp
		${GENERATED_OBJECTS_HEADERS}
		${GENERATED_OBJECTS_SOURCES}
)

target_include_directories(iceobjectsclient
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/Objects
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET iceobjectsclient PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET iceobjectsclient PROPERTY CXX_STANDARD 11)
endif()

if(BUILD_ICE_UWP)
	add_dependencies(iceobjectsclient ${OBJECTS_DEPS})
	set_property(TARGET iceobjectsclient PROPERTY CXX_STANDARD 11)
else()
	target_link_libraries(iceobjectsclient PRIVATE ${OBJECTS_DEPS})
endif()

target_compile_definitions(iceobjectsclient PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Derived.ice
			DerivedEx.ice
			Forward.ice
			Test.ice
	)

	source_group("Source Files/Generated"
		FILES
			${GENERATED_OBJECTS_SOURCES}
	)

	source_group("Header Files/Generated"
		FILES
			${GENERATED_OBJECTS_HEADERS}
	)

	target_compile_options(iceobjectsclient
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	if(NOT BUILD_ICE_UWP)
		set_property(TARGET iceobjectsclient 
			PROPERTY 
				LINK_FLAGS "${ICE_MSVC_DLL_LINK_OPTIONS} /STACK:8388608")
	else()
		set_property(TARGET iceobjectsclient PROPERTY LINK_FLAGS ${ICE_UWP_LINK_OPTIONS})
	endif()
endif()

install(TARGETS iceobjectsclient DESTINATION tests/bin)

add_executable(iceobjectscollocated)

target_sources(iceobjectscollocated
	PRIVATE
		AllTests.cpp
		Collocated.cpp
		TestI.cpp
		TestIntfI.cpp
		${GENERATED_OBJECTS_HEADERS}
		${GENERATED_OBJECTS_SOURCES}
)

target_include_directories(iceobjectscollocated
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/Objects
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_UWP)
	add_dependencies(iceobjectscollocated ${OBJECTS_DEPS})
else()
	target_link_libraries(iceobjectscollocated PRIVATE ${OBJECTS_DEPS})
endif()

target_compile_definitions(iceobjectscollocated PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Derived.ice
			DerivedEx.ice
			Forward.ice
			Test.ice
	)

	source_group("Source Files/Generated"
		FILES
			${GENERATED_OBJECTS_SOURCES}
	)

	source_group("Header Files/Generated"
		FILES
			${GENERATED_OBJECTS_HEADERS}
	)

	target_compile_options(iceobjectscollocated
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	if(NOT BUILD_ICE_UWP)
		set_property(TARGET iceobjectscollocated PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
	else()
		set_property(TARGET iceobjectscollocated PROPERTY LINK_FLAGS ${ICE_UWP_LINK_OPTIONS})
	endif()
endif()

install(TARGETS iceobjectscollocated DESTINATION tests/bin)

add_executable(iceobjectsserver)

target_sources(iceobjectsserver
	PRIVATE
		Server.cpp
		TestI.cpp
		TestIntfI.cpp
		${GENERATED_OBJECTS_HEADERS}
		${GENERATED_OBJECTS_SOURCES}
)

target_include_directories(iceobjectsserver
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/Objects
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET iceobjectsserver PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET iceobjectsserver PROPERTY CXX_STANDARD 11)
endif()

if(BUILD_ICE_UWP)
	add_dependencies(iceobjectsserver ${OBJECTS_DEPS})
	set_property(TARGET iceobjectsserver PROPERTY CXX_STANDARD 11)
else()
	target_link_libraries(iceobjectsserver PRIVATE ${OBJECTS_DEPS})
endif()

target_compile_definitions(iceobjectsserver PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	source_group("Slice Files"
		FILES
			Derived.ice
			DerivedEx.ice
			Forward.ice
			Test.ice
	)

	source_group("Source Files/Generated"
		FILES
			${GENERATED_OBJECTS_SOURCES}
	)

	source_group("Header Files/Generated"
		FILES
			${GENERATED_OBJECTS_HEADERS}
	)

	target_compile_options(iceobjectsserver
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	if(NOT BUILD_ICE_UWP)
		set_property(TARGET iceobjectsserver PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
	else()
		set_property(TARGET iceobjectsserver PROPERTY LINK_FLAGS ${ICE_UWP_LINK_OPTIONS})
	endif()
endif()

install(TARGETS iceobjectsserver DESTINATION tests/bin)
