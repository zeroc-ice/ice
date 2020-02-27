if(NOT ${CMAKE_CURRENT_BINARY_DIR}/Ice)
	file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Ice)
endif()

set(IMPL_SLICES
	${CMAKE_CURRENT_SOURCE_DIR}/Test.ice
	${CMAKE_CURRENT_SOURCE_DIR}/TestAMD.ice
)

list(APPEND ICE_SLICE_DIRS "-I${SLICE_DIR}")

if(BUILD_ICE_CPP98)
	list(APPEND ICE_SLICE_PARAMS ${COMPILE_SLICES_CPP_PARAMETERS} "Ice" "--impl-c++98")
elseif(BUILD_ICE_CPP11)
	list(APPEND ICE_SLICE_PARAMS ${COMPILE_SLICES_CPP_PARAMETERS} "Ice" "--impl-c++11")
endif()

foreach(slice ${IMPL_SLICES})
	get_filename_component(slice_name ${slice} NAME_WE)
	set(output_files 
		${CMAKE_CURRENT_BINARY_DIR}/Ice/${slice_name}.cpp
		${CMAKE_CURRENT_BINARY_DIR}/Ice/${slice_name}I.cpp 
		${CMAKE_CURRENT_BINARY_DIR}/Ice/${slice_name}.h
		${CMAKE_CURRENT_BINARY_DIR}/Ice/${slice_name}I.h
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
	set(IMPL_DEPS 
		ice
		testcommon
	)
elseif(BUILD_ICE_CPP11)
	set(IMPL_DEPS 
		ice++11
		testcommon++11
	)
endif()

add_executable(iceimplserver)

target_sources(iceimplserver
	PRIVATE
		Server.cpp
		${CMAKE_CURRENT_BINARY_DIR}/Ice/Test.cpp
		${CMAKE_CURRENT_BINARY_DIR}/Ice/TestI.cpp
)

target_include_directories(iceimplserver
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/Ice
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET iceimplserver PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET iceimplserver PROPERTY CXX_STANDARD 11)
endif()

target_link_libraries(iceimplserver PRIVATE ${IMPL_DEPS})

target_compile_definitions(iceimplserver 
	PRIVATE 
		${ICE_COMPILE_DEFS}
		"STRING_VIEW_IGNORE_STRING_CONVERTER")

if(MSVC)
	source_group("Slice Files"
		FILES
			Test.ice
	)

	source_group("Source Files/Generated"
		FILES
			${CMAKE_CURRENT_BINARY_DIR}/Ice/Test.cpp
			${CMAKE_CURRENT_BINARY_DIR}/Ice/TestI.cpp
	)

	source_group("Header Files/Generated"
		FILES
			${CMAKE_CURRENT_BINARY_DIR}/Ice/Test.h
			${CMAKE_CURRENT_BINARY_DIR}/Ice/TestI.h
	)

	target_compile_options(iceimplserver
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
			"/wd4100"
			"/wd4503"
	)

	set_property(TARGET iceimplserver PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
endif()

install(TARGETS iceimplserver DESTINATION tests/bin)

add_executable(iceimplserveramd)

target_sources(iceimplserveramd
	PRIVATE
		ServerAMD.cpp
		${CMAKE_CURRENT_BINARY_DIR}/Ice/TestAMD.cpp
		${CMAKE_CURRENT_BINARY_DIR}/Ice/TestAMDI.cpp
)

target_include_directories(iceimplserveramd
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/Ice
		${CMAKE_CURRENT_BINARY_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET iceimplserveramd PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET iceimplserveramd PROPERTY CXX_STANDARD 11)
endif()

target_link_libraries(iceimplserveramd PRIVATE ${IMPL_DEPS})

target_compile_definitions(iceimplserveramd 
	PRIVATE 
		${ICE_COMPILE_DEFS}
		"STRING_VIEW_IGNORE_STRING_CONVERTER")

if(MSVC)
	source_group("Slice Files"
		FILES
			TestAMD.ice
	)

	source_group("Source Files/Generated"
		FILES
			${CMAKE_CURRENT_BINARY_DIR}/Ice/TestAMD.cpp
			${CMAKE_CURRENT_BINARY_DIR}/Ice/TestAMDI.cpp
	)

	source_group("Header Files/Generated"
		FILES
			${CMAKE_CURRENT_BINARY_DIR}/Ice/TestAMD.h
			${CMAKE_CURRENT_BINARY_DIR}/Ice/TestAMDI.h
	)

	target_compile_options(iceimplserveramd
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
			"/wd4100"
			"/wd4503"
	)

	set_property(TARGET iceimplserveramd PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
endif()

install(TARGETS iceimplserveramd DESTINATION tests/bin)
