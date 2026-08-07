# Copyright (c) ZeroC, Inc.

# The version of this Ice installation, read from the installed Ice/Config.h - the header the
# binaries were built with, so it cannot disagree with the installation.
#
#   _ice_package_version      ICE_STRING_VERSION, pre-release suffix included
#   _ice_package_so_version   ICE_SO_VERSION, which forms the Windows library names
#
# Both are left empty when the header is missing, unreadable or carries neither define.

include("${CMAKE_CURRENT_LIST_DIR}/IcePrefix.cmake")

set(_ice_package_version "")
set(_ice_package_so_version "")

if(DEFINED Ice_INCLUDE_ROOT AND EXISTS "${Ice_INCLUDE_ROOT}/Ice/Config.h")
  file(STRINGS "${Ice_INCLUDE_ROOT}/Ice/Config.h" _ice_config_h
    REGEX "^#define ICE_(STRING|SO)_VERSION ")
  if(_ice_config_h MATCHES "#define ICE_STRING_VERSION \"([^\"]+)\"")
    set(_ice_package_version "${CMAKE_MATCH_1}")
  endif()
  if(_ice_config_h MATCHES "#define ICE_SO_VERSION \"([^\"]+)\"")
    set(_ice_package_so_version "${CMAKE_MATCH_1}")
  endif()
  unset(_ice_config_h)
endif()

# All or nothing: a header carrying only one of the two is a broken installation, and both readers
# must agree on that, or the version file accepts a candidate the config file then rejects.
if(NOT _ice_package_version OR NOT _ice_package_so_version)
  set(_ice_package_version "")
  set(_ice_package_so_version "")
endif()
