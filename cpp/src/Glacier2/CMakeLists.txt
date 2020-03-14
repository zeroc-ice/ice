if(NOT BUILD_ICE_CPP11)
	if(NOT ${GENERATED_SLICES_DIR}/Glacier2)
		file(MAKE_DIRECTORY ${GENERATED_SLICES_DIR}/Glacier2)
	endif()

	set(GLACIER2ROUTER_SLICES
		${CMAKE_CURRENT_SOURCE_DIR}/Instrumentation.ice
	)

	set(GENERATED_GLACIER2ROUTER_HEADERS
		${GENERATED_SLICES_DIR}/Glacier2/Instrumentation.h
	)

	set(GENERATED_GLACIER2ROUTER_SOURCES
		${GENERATED_SLICES_DIR}/Glacier2/Instrumentation.cpp
	)

	list(APPEND GLACIER2ROUTER_SLICE_DIRS "-I${SLICE_DIR}")
	list(APPEND GLACIER2ROUTER_SLICE_PARAMS ${COMPILE_SLICES_CPP_PARAMETERS} "Glacier2")

	foreach(slice IN LISTS GLACIER2ROUTER_SLICES)
		#function(CompileSlice slice2bin_params slice_include_paths ice_file output_dir)
		CompileSlice("${GLACIER2ROUTER_SLICE_PARAMS}"
			"${GLACIER2ROUTER_SLICE_DIRS}"
			${slice}
			${GENERATED_SLICES_DIR}/Glacier2
		)
	endforeach()

	install(DIRECTORY ${GENERATED_SLICES_DIR}/Glacier2 
		DESTINATION ${GENERATED_H_INSTALL_DIR}
		FILES_MATCHING PATTERN "*.h"
	)

	add_executable(glacier2router)

	target_sources(glacier2router
		PRIVATE
			Blobject.cpp
			Blobject.h
			ClientBlobject.cpp
			ClientBlobject.h
			FilterI.cpp
			FilterI.h
			FilterManager.cpp
			FilterManager.h
			Glacier2Router.cpp
			Instance.cpp
			Instance.h
			InstrumentationI.cpp
			InstrumentationI.h
			ProxyVerifier.cpp
			ProxyVerifier.h
			RequestQueue.cpp
			RequestQueue.h
			RouterI.cpp
			RouterI.h
			RoutingTable.cpp
			RoutingTable.h
			ServerBlobject.cpp
			ServerBlobject.h
			SessionRouterI.cpp
			SessionRouterI.h
			${GENERATED_GLACIER2ROUTER_HEADERS}
			${GENERATED_GLACIER2ROUTER_SOURCES}
	)

	target_include_directories(glacier2router
		PRIVATE 
			${GENERATED_SLICES_DIR}
			${PROJECT_SOURCE_DIR}/cpp/include
			${PROJECT_SOURCE_DIR}/cpp/src   
	)

	target_compile_definitions(glacier2router
		PRIVATE
			${ICE_COMPILE_DEFS}
	)

	target_link_libraries(glacier2router
		PRIVATE
			glacier2
			icessl
			ice
	)

	if(UNIX)
		target_include_directories(glacier2router PRIVATE ${OPENSSL_INCLUDE_DIR})
		target_link_libraries(glacier2router PRIVATE icesslopenssl)
	endif()

	if(MSVC)
		source_group(
			TREE ${CMAKE_CURRENT_SOURCE_DIR}
			PREFIX "Resource Files"
			FILES
				Glacier2Router.rc
		)

		source_group(
			TREE ${GENERATED_SLICES_DIR}/Glacier2
			PREFIX "Header Files/Generated"
			FILES
				${GENERATED_GLACIER2ROUTER_HEADERS}
		)

		source_group(
			TREE ${GENERATED_SLICES_DIR}/Glacier2
			PREFIX "Source Files/Generated"
			FILES
				${GENERATED_GLACIER2ROUTER_SOURCES}
		)

		source_group(
			TREE ${CMAKE_CURRENT_SOURCE_DIR}
			PREFIX "Slice Files"
			FILES
				${GLACIER2ROUTER_SLICES}
		)

		target_sources(glacier2router
			PRIVATE
				Glacier2Router.rc
		)

		target_compile_options(glacier2router 
			PRIVATE
				${ICE_MSVC_COMPILE_OPTIONS}   
		)

		set_property(TARGET glacier2router 
			PROPERTY 
				LINK_FLAGS "${ICE_MSVC_APP_LINK_OPTIONS} 
					${ICE_MSVC_DLL_LINK_OPTIONS}"
		)
	endif()

	install(TARGETS glacier2router DESTINATION bin)
endif()
