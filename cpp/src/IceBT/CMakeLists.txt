if(NOT BUILD_ICE_CPP11)
	find_package(PkgConfig REQUIRED)
	pkg_check_modules(DBus REQUIRED IMPORTED_TARGET dbus-1)

	if(NOT ${GENERATED_SLICES_DIR}/IceBT)
		file(MAKE_DIRECTORY ${GENERATED_SLICES_DIR}/IceBT)
	endif()

	set(GENERATED_ICEBT_HEADERS
		${GENERATED_SLICES_DIR}/IceBT/ConnectionInfo.h
		${GENERATED_SLICES_DIR}/IceBT/EndpointInfo.h
		${GENERATED_SLICES_DIR}/IceBT/Types.h
	)

	set(GENERATED_ICEBT_SOURCES
		${GENERATED_SLICES_DIR}/IceBT/ConnectionInfo.cpp
		${GENERATED_SLICES_DIR}/IceBT/EndpointInfo.cpp
		${GENERATED_SLICES_DIR}/IceBT/Types.cpp
	)

	list(APPEND ICEBT_SLICE_DIRS "-I${SLICE_DIR}")
	list(APPEND ICEBT_SLICE_PARAMS ${COMPILE_SLICES_CPP_PARAMETERS} "IceBT")

	foreach(slice IN LISTS ICEBT_SLICES)
		#function(CompileSlice slice2bin_params slice_include_paths ice_file output_dir)
		CompileSlice("${ICEBT_SLICE_PARAMS}"
			"${ICEBT_SLICE_DIRS}"
			${slice}
			${GENERATED_SLICES_DIR}/IceBT
		)
	endforeach()

	install(DIRECTORY ${GENERATED_SLICES_DIR}/IceBT 
		DESTINATION ${GENERATED_H_INSTALL_DIR}
		FILES_MATCHING PATTERN "*.h"
	)	

	add_library(icebt)

	target_sources(icebt
		PRIVATE
			AcceptorI.cpp
			AcceptorI.h
			ConnectorI.cpp
			ConnectorI.h
			DBus.cpp
			DBus.h
			EndpointI.cpp
			EndpointI.h
			Engine.cpp
			Engine.h
			EngineF.h
			Instance.cpp
			Instance.h
			PluginI.cpp
			PluginI.h
			StreamSocket.cpp
			StreamSocket.h
			TransceiverI.cpp
			TransceiverI.h
			Util.cpp
			Util.h
			${GENERATED_ICEBT_HEADERS}
			${GENERATED_ICEBT_SOURCES}
	)

	target_include_directories(icebt
		PRIVATE 
			${GENERATED_SLICES_DIR}
			${PROJECT_SOURCE_DIR}/cpp/include
			${PROJECT_SOURCE_DIR}/cpp/src
			${DBUS_INCLUDE_DIRS}
	)

	target_compile_definitions(icebt
		PRIVATE
			${ICE_COMPILE_DEFS}
			"ICEBT_API_EXPORTS"
	)

	target_link_libraries(icebt
		PRIVATE
			ice
			PkgConfig::DBus
	)

	set_target_properties(icebt
		PROPERTIES
			OUTPUT_NAME IceBT${zeroc-ice_VERSION_MAJOR}${zeroc-ice_VERSION_MINOR}
	)

	install(TARGETS icebt DESTINATION lib)
endif()
