if(BUILD_ICE_CPP98)
	set(PROPERTIES_DEPS 
		ice
		testcommon
	)
elseif(BUILD_ICE_CPP11)
	set(PROPERTIES_DEPS 
		ice++11
		testcommon++11
	)
endif()

add_executable(icepropertiesclient)

target_sources(icepropertiesclient
	PRIVATE
		Client.cpp
)

target_include_directories(icepropertiesclient
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/src
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${CMAKE_CURRENT_SOURCE_DIR}
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET icepropertiesclient PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET icepropertiesclient PROPERTY CXX_STANDARD 11)
endif()

target_link_libraries(icepropertiesclient PRIVATE ${PROPERTIES_DEPS})

target_compile_definitions(icepropertiesclient PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	target_compile_options(icepropertiesclient
		PRIVATE
			${ICE_MSVC_COMPILE_OPTIONS}
	)

	set_property(TARGET icepropertiesclient PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
endif()

install(TARGETS icepropertiesclient DESTINATION tests/bin)
