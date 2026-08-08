# Copyright (c) ZeroC, Inc.

# CMake package configuration file for Ice.

# A previous find_package(Ice) in this scope may have left one behind.
unset(Ice_NOT_FOUND_MESSAGE)

# Slice dependency tracking uses add_custom_command(DEPFILE), which the Xcode and Visual Studio
# generators only support from 3.21.
if(CMAKE_VERSION VERSION_LESS 3.21)
  set(Ice_FOUND FALSE)
  set(Ice_NOT_FOUND_MESSAGE "Ice requires CMake 3.21 or later, but this is CMake ${CMAKE_VERSION}.")
  return()
endif()

include("${CMAKE_CURRENT_LIST_DIR}/IcePrefix.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/IceVersion.cmake")

if(NOT DEFINED Ice_PREFIX)
  set(Ice_FOUND FALSE)
  string(CONCAT Ice_NOT_FOUND_MESSAGE
    "Could not locate the Ice installation: ${CMAKE_CURRENT_LIST_DIR} is not in a recognized "
    "installation layout, or the installation is missing Ice/Ice.h.")
  return()
endif()

if(NOT Ice_VERSION)
  set(Ice_FOUND FALSE)
  string(CONCAT Ice_NOT_FOUND_MESSAGE
    "Could not read ICE_STRING_VERSION and ICE_SO_VERSION from "
    "'${Ice_INCLUDE_ROOT}/Ice/Config.h'.")
  return()
endif()

# find_package sets Ice_CONFIG only once this file returns, but find_package_handle_standard_args
# needs it in CONFIG_MODE while IceTargets.cmake runs.
set(Ice_CONFIG "${CMAKE_CURRENT_LIST_FILE}")

include("${CMAKE_CURRENT_LIST_DIR}/IceTargets.cmake")

# Internal to the package; Ice_PREFIX is the documented result variable.
unset(Ice_INCLUDE_ROOT)

include("${CMAKE_CURRENT_LIST_DIR}/slice2cpp.cmake")
