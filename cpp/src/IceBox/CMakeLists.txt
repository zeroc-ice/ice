if(NOT ${GENERATED_SLICES_DIR}/IceBox)
	file(MAKE_DIRECTORY ${GENERATED_SLICES_DIR}/IceBox)
endif()

set(GENERATED_ICEBOXLIB_HEADERS
	${GENERATED_SLICES_DIR}/IceBox/IceBox.h
)

set(GENERATED_ICEBOXLIB_SOURCES
	${GENERATED_SLICES_DIR}/IceBox/IceBox.cpp
)

set(ICEBOXLIB_SOURCES
	Exception.cpp
)

set(ICEBOX_SOURCES
	Service.cpp
	ServiceManagerI.cpp
	ServiceManagerI.h
)

list(APPEND ICEBOXLIB_SLICE_DIRS "-I${SLICE_DIR}")
list(APPEND ICEBOXLIB_SLICE_PARAMS ${COMPILE_SLICES_CPP_PARAMETERS} "IceBox")

foreach(slice IN LISTS ICEBOXLIB_SLICES)
	#function(CompileSlice slice2bin_params slice_include_paths ice_file output_dir)
	CompileSlice("${ICEBOXLIB_SLICE_PARAMS}"
		"${ICEBOXLIB_SLICE_DIRS}"
		${slice}
		${GENERATED_SLICES_DIR}/IceBox
	)
endforeach()

install(DIRECTORY ${GENERATED_SLICES_DIR}/IceBox 
	DESTINATION ${GENERATED_H_INSTALL_DIR}
	FILES_MATCHING PATTERN "*.h"
)

if(BUILD_ICE_CPP11)
	set(ICEBOXLIB_TARGET iceboxlib++11)

	add_library(${ICEBOXLIB_TARGET})

	set_target_properties(${ICEBOXLIB_TARGET} 
		PROPERTIES
			OUTPUT_NAME IceBox${zeroc-ice_VERSION_MAJOR}${zeroc-ice_VERSION_MINOR}++11
			CXX_STANDARD 11
	)

	target_link_libraries(${ICEBOXLIB_TARGET} PRIVATE ice++11)
else()
	set(ICEBOXLIB_TARGET iceboxlib)

	add_library(${ICEBOXLIB_TARGET})

	set_target_properties(${ICEBOXLIB_TARGET} 
		PROPERTIES
			OUTPUT_NAME IceBox${zeroc-ice_VERSION_MAJOR}${zeroc-ice_VERSION_MINOR}
	)

	target_link_libraries(${ICEBOXLIB_TARGET} PRIVATE ice)
endif()

target_sources(${ICEBOXLIB_TARGET}
	PRIVATE
		${ICEBOXLIB_SOURCES}
		${GENERATED_ICEBOXLIB_HEADERS}
		${GENERATED_ICEBOXLIB_SOURCES}
)

target_include_directories(${ICEBOXLIB_TARGET}
	PRIVATE 
		${GENERATED_SLICES_DIR}
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/src
)

target_compile_definitions(${ICEBOXLIB_TARGET} 
	PRIVATE 
		${ICE_COMPILE_DEFS}
		"ICEBOX_API_EXPORTS"
)

if(MSVC)
	source_group(
		TREE ${CMAKE_CURRENT_SOURCE_DIR}
		PREFIX "Resource Files"
		FILES
			IceBox.rc
	)

	source_group(
		TREE ${GENERATED_SLICES_DIR}/IceBox
		PREFIX "Header Files/Generated"
		FILES
			${GENERATED_ICEBOXLIB_HEADERS}
	)

	source_group(
		TREE ${GENERATED_SLICES_DIR}/IceBox
		PREFIX "Source Files/Generated"
		FILES
			${GENERATED_ICEBOXLIB_SOURCES}
	)

	source_group(
		TREE ${SLICE_DIR}/IceBox
		PREFIX "Slice Files"
		FILES
			${ICEBOXLIB_SLICES}
	)

	target_sources(${ICEBOXLIB_TARGET}
		PRIVATE
			IceBox.rc
	)

	set_property(TARGET ${ICEBOXLIB_TARGET} PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})

	target_compile_options(${ICEBOXLIB_TARGET} 
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}   
	)
endif()

if(BUILD_SHARED_LIBS)
	install(TARGETS ${ICEBOXLIB_TARGET} DESTINATION bin)
else()
	install(TARGETS ${ICEBOXLIB_TARGET} DESTINATION lib)
endif()

if(BUILD_ICE_CPP11)
	set(ICEBOX_TARGET icebox++11)

	add_executable(${ICEBOX_TARGET})

	set_target_properties(${ICEBOX_TARGET} 
		PROPERTIES
			OUTPUT_NAME icebox
			CXX_STANDARD 11
	)

	target_link_libraries(${ICEBOX_TARGET}
		PRIVATE
			iceboxlib++11
			ice++11
	)
else()
	set(ICEBOX_TARGET icebox)

	add_executable(${ICEBOX_TARGET})

	set_target_properties(${ICEBOX_TARGET} PROPERTIES OUTPUT_NAME icebox)

	target_link_libraries(${ICEBOX_TARGET}
		PRIVATE
			iceboxlib
			ice
	)
endif()

target_sources(${ICEBOX_TARGET} 
	PRIVATE
		${ICEBOX_SOURCES}
)

target_include_directories(${ICEBOX_TARGET}
	PRIVATE
		${GENERATED_SLICES_DIR}
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/src
)

target_compile_definitions(${ICEBOX_TARGET}
	PRIVATE
		${ICE_COMPILE_DEFS}
)

if(MSVC)
	source_group(
		TREE ${CMAKE_CURRENT_SOURCE_DIR}
		PREFIX "Resource Files"
		FILES
			IceBoxExe.rc
	)

	target_sources(${ICEBOX_TARGET}
		PRIVATE
			IceBoxExe.rc
	)

	set_property(TARGET ${ICEBOX_TARGET} 
		PROPERTY
			LINK_FLAGS 
				"${ICE_MSVC_DLL_LINK_OPTIONS} 
				${ICE_MSVC_APP_LINK_OPTIONS}"
	)

	target_compile_options(${ICEBOX_TARGET}
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)
endif()

install(TARGETS ${ICEBOX_TARGET} DESTINATION bin)

if(NOT BUILD_ICE_CPP11)
	add_executable(iceboxadmin)

	target_sources(iceboxadmin
		PRIVATE
			Admin.cpp
	)

	target_include_directories(iceboxadmin
		PRIVATE
			${GENERATED_SLICES_DIR}
			${PROJECT_SOURCE_DIR}/cpp/include
			${PROJECT_SOURCE_DIR}/cpp/src
	)

	target_compile_definitions(iceboxadmin
		PRIVATE
			${ICE_COMPILE_DEFS}
	)

	target_link_libraries(iceboxadmin 
		PRIVATE 
			iceboxlib
			ice
	)

	if(MSVC)
		source_group(
			TREE ${CMAKE_CURRENT_SOURCE_DIR}
			PREFIX "Resource Files"
			FILES
				IceBoxAdmin.rc
		)

		target_sources(iceboxadmin
			PRIVATE
				IceBoxAdmin.rc
		)

		set_property(TARGET iceboxadmin 
			PROPERTY
				LINK_FLAGS 
					"${ICE_MSVC_DLL_LINK_OPTIONS} 
					${ICE_MSVC_APP_LINK_OPTIONS}"
		)

		target_compile_options(iceboxadmin
			PRIVATE
				${ICE_MSVC_COMPILE_OPTIONS}
		)
	endif()

	install(TARGETS iceboxadmin DESTINATION bin)
endif()
