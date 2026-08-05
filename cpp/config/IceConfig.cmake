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

# OPTIONAL with a guard: a sibling file missing from a mangled installation must report Ice as not
# found, not abort the configure step - include() of a missing file is a hard error even under QUIET.
include("${CMAKE_CURRENT_LIST_DIR}/IcePrefix.cmake" OPTIONAL RESULT_VARIABLE _ice_prefix_included)
include("${CMAKE_CURRENT_LIST_DIR}/IceVersion.cmake" OPTIONAL RESULT_VARIABLE _ice_version_included)

if(NOT _ice_prefix_included OR NOT _ice_version_included)
  set(Ice_FOUND FALSE)
  set(Ice_NOT_FOUND_MESSAGE
    "The Ice installation at ${CMAKE_CURRENT_LIST_DIR} is missing IcePrefix.cmake or IceVersion.cmake.")
  unset(_ice_prefix_included)
  unset(_ice_version_included)
  return()
endif()
unset(_ice_prefix_included)
unset(_ice_version_included)

set(Ice_SO_VERSION "${_ice_package_so_version}")

# Unconditionally: under find_package this matches what IceConfigVersion.cmake reported, and a
# leftover cache entry from an older installation must not win over the installed version.
set(Ice_VERSION "${_ice_package_version}")

unset(_ice_package_version)
unset(_ice_package_so_version)

if(NOT DEFINED Ice_PREFIX)
  set(Ice_FOUND FALSE)
  set(Ice_NOT_FOUND_MESSAGE
    "Could not locate the Ice installation: ${CMAKE_CURRENT_LIST_DIR} is not in a recognized "
    "installation layout, or the installation is missing Ice/Ice.h.")
  return()
endif()

# find_package sets Ice_CONFIG only once this file returns, but find_package_handle_standard_args
# needs it in CONFIG_MODE while IceTargets.cmake runs.
set(Ice_CONFIG "${CMAKE_CURRENT_LIST_FILE}")

include("${CMAKE_CURRENT_LIST_DIR}/IceTargets.cmake")

# Internal to IcePrefix/IceTargets; Ice_PREFIX is the documented result variable.
unset(Ice_INCLUDE_ROOT)

include("${CMAKE_CURRENT_LIST_DIR}/slice2cpp.cmake")
