if(BUILD_ICE_CPP11)
	set(TESTCOMMON_TARGET testcommon++11)
	
	set(TESTCOMMON_DEPS 
		ice++11
		icessl++11
	)

	add_library(${TESTCOMMON_TARGET})
	set_target_properties(${TESTCOMMON_TARGET} 
		PROPERTIES
			OUTPUT_NAME testcommon${zeroc-ice_VERSION_MAJOR}${zeroc-ice_VERSION_MINOR}++11
			CXX_STANDARD 11
	)
else()
	set(TESTCOMMON_TARGET testcommon)
	
	set(TESTCOMMON_DEPS 
		ice
		icessl
	)
	add_library(${TESTCOMMON_TARGET})
	set_target_properties(${TESTCOMMON_TARGET} 
		PROPERTIES
			OUTPUT_NAME testcommon${zeroc-ice_VERSION_MAJOR}${zeroc-ice_VERSION_MINOR}
	)
endif()

target_sources(${TESTCOMMON_TARGET} PRIVATE TestHelper.cpp)

target_include_directories(${TESTCOMMON_TARGET}
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${GENERATED_SLICES_DIR}
)

target_compile_definitions(${TESTCOMMON_TARGET}
	PRIVATE
		${ICE_COMPILE_DEFS}
		"TEST_API_EXPORTS"
)

if(NOT BUILD_ICE_UWP)
	target_link_libraries(${TESTCOMMON_TARGET} PRIVATE ${TESTCOMMON_DEPS})
else()
	add_dependencies(${TESTCOMMON_TARGET} ${TESTCOMMON_DEPS})
endif()

if(MSVC)
	set_property(TARGET ${TESTCOMMON_TARGET} PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
	target_compile_options(${TESTCOMMON_TARGET} PRIVATE ${ICE_MSVC_COMPILE_OPTIONS})
endif()

if(BUILD_SHARED_LIBS)
	install(TARGETS ${TESTCOMMON_TARGET} DESTINATION test/bin)
else()
	install(TARGETS ${TESTCOMMON_TARGET} DESTINATION test/lib)
endif()
