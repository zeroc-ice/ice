if(NOT ${GENERATED_SLICES_DIR}/IceStorm)
	file(MAKE_DIRECTORY ${GENERATED_SLICES_DIR}/IceStorm)
endif()

set(ICESTORM_LOCAL_SLICES
	${CMAKE_CURRENT_SOURCE_DIR}/DBTypes.ice
	${CMAKE_CURRENT_SOURCE_DIR}/Election.ice
	${CMAKE_CURRENT_SOURCE_DIR}/IceStormInternal.ice
	${CMAKE_CURRENT_SOURCE_DIR}/Instrumentation.ice
	${CMAKE_CURRENT_SOURCE_DIR}/LinkRecord.ice
	${CMAKE_CURRENT_SOURCE_DIR}/LLURecord.ice
	${CMAKE_CURRENT_SOURCE_DIR}/SubscriberRecord.ice
)

set(GENERATED_LOCAL_ICESTORM_HEADERS
	${GENERATED_SLICES_DIR}/IceStorm/Election.h
	${GENERATED_SLICES_DIR}/IceStorm/IceStormInternal.h
	${GENERATED_SLICES_DIR}/IceStorm/Instrumentation.h
	${GENERATED_SLICES_DIR}/IceStorm/LinkRecord.h
	${GENERATED_SLICES_DIR}/IceStorm/LLURecord.h
	${GENERATED_SLICES_DIR}/IceStorm/SubscriberRecord.h
)

set(GENERATED_LOCAL_ICESTORM_SOURCES
	${GENERATED_SLICES_DIR}/IceStorm/Election.cpp
	${GENERATED_SLICES_DIR}/IceStorm/IceStormInternal.cpp
	${GENERATED_SLICES_DIR}/IceStorm/Instrumentation.cpp
	${GENERATED_SLICES_DIR}/IceStorm/LinkRecord.cpp
	${GENERATED_SLICES_DIR}/IceStorm/LLURecord.cpp
	${GENERATED_SLICES_DIR}/IceStorm/SubscriberRecord.cpp
)

set(GENERATED_ICESTORMDB_HEADERS
	${GENERATED_SLICES_DIR}/IceStorm/DBTypes.h
	${GENERATED_SLICES_DIR}/IceStorm/LLURecord.h
	${GENERATED_SLICES_DIR}/IceStorm/SubscriberRecord.h
)

set(GENERATED_ICESTORMDB_SOURCES
	${GENERATED_SLICES_DIR}/IceStorm/DBTypes.cpp
	${GENERATED_SLICES_DIR}/IceStorm/LLURecord.cpp
	${GENERATED_SLICES_DIR}/IceStorm/SubscriberRecord.cpp
)

list(APPEND ICESTORM_SLICE_DIRS "-I${SLICE_DIR}" "-I${PROJECT_SOURCE_DIR}/cpp/src")
list(APPEND ICESTORM_SLICE_PARAMS ${COMPILE_SLICES_CPP_PARAMETERS} "IceStorm")

foreach(slice IN LISTS ICESTORM_LOCAL_SLICES)
	#function(CompileSlice slice2bin_params slice_include_paths ice_file output_dir)
	CompileSlice("${ICESTORM_SLICE_PARAMS}"
		"${ICESTORM_SLICE_DIRS}"
		${slice}
		${GENERATED_SLICES_DIR}/IceStorm
	)
endforeach()

install(DIRECTORY ${GENERATED_SLICES_DIR}/IceStorm 
	DESTINATION ${GENERATED_H_INSTALL_DIR}
	FILES_MATCHING PATTERN "*.h"
)

add_executable(icestormadmin)

target_sources(icestormadmin
	PRIVATE
		Admin.cpp
		Grammar.cpp
		Grammar.h
		Parser.cpp
		Parser.h
		Scanner.cpp
		${GENERATED_LOCAL_ICESTORM_HEADERS}
		${GENERATED_LOCAL_ICESTORM_SOURCES}
)

target_include_directories(icestormadmin
	PRIVATE 
		${GENERATED_SLICES_DIR}
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/src
)

target_compile_definitions(icestormadmin
	PRIVATE
		${ICE_COMPILE_DEFS}
)

target_link_libraries(icestormadmin 
	PRIVATE
		icestorm
		ice
)

if(MSVC)
	source_group(
		TREE ${CMAKE_CURRENT_SOURCE_DIR}
		PREFIX "Resource Files"
		FILES
			IceStormAdmin.rc
	)

	source_group(
		TREE ${GENERATED_SLICES_DIR}/IceStorm
		PREFIX "Header Files/Generated"
		FILES
			${GENERATED_LOCAL_ICESTORM_HEADERS}
	)

	source_group(
		TREE ${GENERATED_SLICES_DIR}/IceStorm
		PREFIX "Source Files/Generated"
		FILES
			${GENERATED_LOCAL_ICESTORM_SOURCES}
	)

	source_group(
		TREE ${CMAKE_CURRENT_SOURCE_DIR}
		PREFIX "Slice Files"
		FILES
			${ICESTORM_LOCAL_SLICES}
	)

	target_sources(icestormadmin
		PRIVATE
			IceStormAdmin.rc
	)

	target_compile_options(icestormadmin
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}   
	)

	set_property(TARGET icestormadmin 
		PROPERTY 
			LINK_FLAGS "${ICE_MSVC_APP_LINK_OPTIONS} 
				${ICE_MSVC_DLL_LINK_OPTIONS}"
	)
endif()

install(TARGETS icestormadmin DESTINATION bin)

add_executable(icestormdb)

add_dependencies(icestormdb icestormservice)

target_sources(icestormdb
	PRIVATE
		IceStormDB.cpp
		${GENERATED_ICESTORMDB_HEADERS}
		${GENERATED_ICESTORMDB_SOURCES}
)

target_include_directories(icestormdb
	PRIVATE 
		${GENERATED_SLICES_DIR}
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/src
		${LMDB_INCLUDE_DIR}
)

target_compile_definitions(icestormdb
	PRIVATE
		${ICE_COMPILE_DEFS}
)

target_link_libraries(icestormdb 
	PRIVATE
		icestorm
		ice
		icepatch2
		icedb
)

if(MSVC)
	source_group(
		TREE ${CMAKE_CURRENT_SOURCE_DIR}
		PREFIX "Resource Files"
		FILES
			IceStormDB.rc
	)

	source_group(
		TREE ${GENERATED_SLICES_DIR}/IceStorm
		PREFIX "Header Files/Generated"
		FILES
			${GENERATED_ICESTORMDB_HEADERS}
	)

	source_group(
		TREE ${GENERATED_SLICES_DIR}/IceStorm
		PREFIX "Source Files/Generated"
		FILES
			${GENERATED_ICESTORMDB_SOURCES}
	)

	source_group(
		TREE ${CMAKE_CURRENT_SOURCE_DIR}
		PREFIX "Slice Files"
		FILES
			DBTypes.ice
			Election.ice
			IceStormInternal.ice
	)

	target_sources(icestormdb
		PRIVATE
			IceStormDB.rc
	)

	target_compile_options(icestormdb
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}   
	)

	set_property(TARGET icestormdb 
		PROPERTY 
			LINK_FLAGS "${ICE_MSVC_APP_LINK_OPTIONS} 
				${ICE_MSVC_DLL_LINK_OPTIONS}"
	)		
endif()

install(TARGETS icestormdb DESTINATION bin)

add_library(icestormservice)

target_sources(icestormservice
	PRIVATE
		Instance.cpp
		Instance.h
		InstrumentationI.cpp
		InstrumentationI.h
		NodeI.cpp
		NodeI.h
		Observers.cpp
		Observers.h
		Replica.h
		Service.cpp
		Service.h
		Subscriber.cpp
		Subscriber.h
		TopicI.cpp
		TopicI.h
		TopicManagerI.cpp
		TopicManagerI.h
		TraceLevels.cpp
		TraceLevels.h
		TransientTopicI.cpp
		TransientTopicI.h
		TransientTopicManagerI.cpp
		TransientTopicManagerI.h
		Util.cpp
		Util.h
		${GENERATED_LOCAL_ICESTORM_HEADERS}
		${GENERATED_LOCAL_ICESTORM_SOURCES}
)

target_include_directories(icestormservice
	PRIVATE 
		${GENERATED_SLICES_DIR}
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/src
		${LMDB_INCLUDE_DIR}
)

target_compile_definitions(icestormservice
	PRIVATE
		${ICE_COMPILE_DEFS}
)

target_link_libraries(icestormservice
	PRIVATE
		icestorm
		ice
		icegrid
		glacier2
		iceboxlib
		icedb
	)

if(MSVC)
	source_group(
		TREE ${CMAKE_CURRENT_SOURCE_DIR}
		PREFIX "Resource Files"
		FILES
			IceStormService.rc
	)

	source_group(
		TREE ${GENERATED_SLICES_DIR}/IceStorm
		PREFIX "Header Files/Generated"
		FILES
			${GENERATED_LOCAL_ICESTORM_HEADERS}
	)

	source_group(
		TREE ${GENERATED_SLICES_DIR}/IceStorm
		PREFIX "Source Files/Generated"
		FILES
			${GENERATED_LOCAL_ICESTORM_SOURCES}
	)

	source_group(
		TREE ${CMAKE_CURRENT_SOURCE_DIR}
		PREFIX "Slice Files"
		FILES
			${ICESTORM_LOCAL_SLICES}
	)

	target_sources(icestormservice
		PRIVATE
			IceStormService.rc
	)

	set_property(TARGET icestormservice PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})

	target_compile_options(icestormservice
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}   
	)
endif()

set_target_properties(icestormservice
	PROPERTIES
		OUTPUT_NAME IceStormService${zeroc-ice_VERSION_MAJOR}${zeroc-ice_VERSION_MINOR}
)

if(BUILD_SHARED_LIBS)
	install(TARGETS icestormservice DESTINATION bin)
else()
	install(TARGETS icestormservice DESTINATION lib)
endif()
