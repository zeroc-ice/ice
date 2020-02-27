if(BUILD_ICE_CPP98)
	set(PERSISTENT_DEPS 
		ice
		icestorm
		testcommon
	)
elseif(BUILD_ICE_CPP11)
	set(PERSISTENT_DEPS 
		ice++11
		icestorm++11
		testcommon++11
	)
endif()

add_executable(icestormpersistentclient)

target_sources(icestormpersistentclient PRIVATE Client.cpp)

target_include_directories(icestormpersistentclient
	PRIVATE
		${PROJECT_SOURCE_DIR}/cpp/include
		${PROJECT_SOURCE_DIR}/cpp/test/include
		${GENERATED_SLICES_DIR}
)

if(BUILD_ICE_CPP98)
	set_property(TARGET icestormpersistentclient PROPERTY CXX_STANDARD 98)
elseif(BUILD_ICE_CPP11)
	set_property(TARGET icestormpersistentclient PROPERTY CXX_STANDARD 11)
endif()

target_link_libraries(icestormpersistentclient PRIVATE ${PERSISTENT_DEPS})

target_compile_definitions(icestormpersistentclient PRIVATE ${ICE_COMPILE_DEFS})

if(MSVC)
	set_property(TARGET icestormpersistentclient PROPERTY LINK_FLAGS ${ICE_MSVC_DLL_LINK_OPTIONS})
endif()

install(TARGETS icestormpersistentclient DESTINATION tests/bin)
