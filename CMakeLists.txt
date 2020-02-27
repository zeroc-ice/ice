cmake_minimum_required(VERSION 3.7.2)

option(BUILD_ICE_TESTS OFF)
option(BUILD_ICE_CXX OFF)
option(BUILD_ICE_CS OFF)

# handle debug lib naming based on shipped project naming
if(NOT CMAKE_DEBUG_POSTFIX)
	set(CMAKE_DEBUG_POSTFIX d)
endif()

if(BUILD_ICE_CXX)
	project(zeroc-ice CXX)
	enable_language(CXX)
elseif(BUILD_ICE_CS)
	project(zeroc-ice CSharp)
	enable_language(CSharp)
endif()

set(zeroc-ice_VERSION_MAJOR 3)
set(zeroc-ice_VERSION_MINOR 7)
set(zeroc-ice_VERSION_PATCH 3)

if(CMAKE_CONFIGURATION_TYPES)
	if(BUILD_ICE_TESTS)
		set(CMAKE_CONFIGURATION_TYPES "Debug" CACHE STRING "" FORCE)
	else()
		set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)
	endif()
endif()

# create root path for .ice files
set(SLICE_DIR ${CMAKE_SOURCE_DIR}/slice)

# create include/generated folder for generated headers
if(NOT ${PROJECT_BINARY_DIR}/generated)
	file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/generated)
endif()

# generated sources build dir
set(GENERATED_SLICES_DIR ${PROJECT_BINARY_DIR}/generated)

# add project wide slices
include(cmake/Slices.cmake)

# add function to handle slice compiles (icebuilder)
include(cmake/CompileSlice.cmake)

# add icebuilder external project
include(ExternalProject)

find_library(MCPP_LIB_PATH NAMES mcpp)

ExternalProject_Add(icebuilder
	DOWNLOAD_COMMAND ""
	CONFIGURE_COMMAND ""
	BUILD_COMMAND ""	
	INSTALL_COMMAND ""
)

# do a compiler language check
get_property(languages GLOBAL PROPERTY ENABLED_LANGUAGES)

# check language type, add subdirectories
if("${languages}" MATCHES "CXX")
	execute_process(COMMAND ${CMAKE_COMMAND} 
		-G ${CMAKE_GENERATOR}
		${CMAKE_SOURCE_DIR}/icebuilder
		-DBUILD_ICEBUILDER_CXX=ON
		-DICE_SOURCES_ROOT=${CMAKE_SOURCE_DIR}
		-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/icebuilder
		-DMCPP_LIB=${MCPP_LIB_PATH}
		WORKING_DIRECTORY 
			${CMAKE_BINARY_DIR}/icebuilder-prefix/src
	)

	if(CMAKE_GENERATOR MATCHES "Visual Studio")
		execute_process(COMMAND ${CMAKE_COMMAND} --build 
			${CMAKE_BINARY_DIR}/icebuilder-prefix/src
			--config Release --target INSTALL
		)
	else()
		execute_process(COMMAND ${CMAKE_COMMAND} --build 
			${CMAKE_BINARY_DIR}/icebuilder-prefix/src
			--target install
		)
	endif()

	add_subdirectory(cpp)
elseif ("${languages}" MATCHES "CSharp")
	add_subdirectory(csharp)
elseif("${languages" MATCHES "Java")
	add_subdirectory(java)
endif()
