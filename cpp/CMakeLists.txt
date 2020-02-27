# set build types
option(BUILD_ICE_CPP11 OFF)

# generated headers install dir
set(GENERATED_H_INSTALL_DIR ${PROJECT_BINARY_DIR}/include)

# add project wide c++ properties
include(cmake/ice_cpp_props.cmake)

if(MSVC)
	include(cmake/ice_msvc_props.cmake)
endif()

set(zeroc-ice_DOXYGEN_FILES
	doxygen/config-cpp11
	doxygen/config-cpp98
	doxygen/mainpage-cpp11.md
	doxygen/mainpage-cpp98.md
)

set(zeroc-ice_CONFIG_FILES
	config/glacer2router.cfg
	config/ice_ca.cnf
	config/icegridnode.cfg
	config/icegridregistry.cfg
	config/templates.xml
)

set_source_files_properties(
	${zeroc-ice_CONFIG_FILES} 
	${zeroc-ice_DOXYGEN_FILES}
	PROPERTIES HEADER_FILE_ONLY TRUE
)

find_program(SLICE2_BIN slice2cpp HINTS ${PROJECT_BINARY_DIR}/icebuilder/bin)

find_package(BZip2 REQUIRED)
find_package(EXPAT REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)

if(NOT BUILD_ICE_CPP11)
	find_package(lmdb CONFIG REQUIRED)
endif()

add_subdirectory(src/IceUtil)
add_subdirectory(src/Ice)
add_subdirectory(src/Glacier2Lib)
add_subdirectory(src/IceBox)
add_subdirectory(src/IceDiscovery)
add_subdirectory(src/IceGridLib)
add_subdirectory(src/IceLocatorDiscovery)
add_subdirectory(src/IceSSL)
add_subdirectory(src/IceStormLib)
add_subdirectory(src/IceGrid)

if(NOT BUILD_ICE_CPP11)
	add_subdirectory(src/Glacier2)
	add_subdirectory(src/Glacier2CryptPermissionsVerifier)
	add_subdirectory(src/IceBridge)
	add_subdirectory(src/IceDB)
	add_subdirectory(src/icegriddb)
	add_subdirectory(src/IceXML)
	add_subdirectory(src/IcePatch2)
	add_subdirectory(src/IcePatch2Lib)
	add_subdirectory(src/IceStorm)
	add_subdirectory(src/Slice)
	
	if(MSVC)
		add_subdirectory(src/iceserviceinstall)
	endif()
	if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
		add_subdirectory(src/IceBT)
	endif()
endif()

if(BUILD_ICE_TESTS)
	add_subdirectory(test)
endif()
