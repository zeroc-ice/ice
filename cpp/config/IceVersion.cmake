# Copyright (c) ZeroC, Inc.

# The version of this Ice installation, read from the installed Ice/Config.h - the header the
# binaries were built with, so it cannot disagree with the installation.
#
#   Ice_VERSION      ICE_STRING_VERSION, pre-release suffix included
#   Ice_SO_VERSION   ICE_SO_VERSION, which forms the Windows library names
#
# Both are always assigned, and left empty when the header is missing, unreadable or does not
# carry both defines.
#
# Reads Ice_INCLUDE_ROOT, so include IcePrefix.cmake first.

set(Ice_VERSION "")
set(Ice_SO_VERSION "")

if(DEFINED Ice_INCLUDE_ROOT AND EXISTS "${Ice_INCLUDE_ROOT}/Ice/Config.h")
  file(STRINGS "${Ice_INCLUDE_ROOT}/Ice/Config.h" _ice_config_h
    REGEX "^#define ICE_(STRING|SO)_VERSION ")
  if(_ice_config_h MATCHES "#define ICE_STRING_VERSION \"([^\"]+)\"")
    set(Ice_VERSION "${CMAKE_MATCH_1}")
  endif()
  if(_ice_config_h MATCHES "#define ICE_SO_VERSION \"([^\"]+)\"")
    set(Ice_SO_VERSION "${CMAKE_MATCH_1}")
  endif()
  unset(_ice_config_h)
endif()

# All or nothing: a header carrying only one of the two is a broken installation, and both readers
# must agree on that, or the version file accepts a candidate the config file then rejects.
if(NOT Ice_VERSION OR NOT Ice_SO_VERSION)
  set(Ice_VERSION "")
  set(Ice_SO_VERSION "")
endif()
