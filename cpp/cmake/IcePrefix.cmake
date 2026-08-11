# Copyright (c) ZeroC, Inc.

# Locates the Ice installation prefix from this file's own location. The packaging layouts are:
#
#   <prefix>/cmake/IceConfig.cmake                              (Windows NuGet package)
#   <prefix>/lib*/cmake/Ice-<mmversion>/IceConfig.cmake         (macOS, RHEL, single-arch Linux)
#   <prefix>/lib*/<arch>/cmake/Ice-<mmversion>/IceConfig.cmake  (Debian/Ubuntu multiarch)
#
# The layout decides where the prefix is, and the headers are then required exactly there: a broken
# installation reports not found rather than adopting an enclosing one that happens to have headers,
# and the depth never depends on the consumer's CMAKE_LIBRARY_ARCHITECTURE.
#
# Sets Ice_PREFIX and Ice_INCLUDE_ROOT, or leaves both unset. Runs at include time so that
# CMAKE_CURRENT_LIST_DIR is this file's directory; in a function it would be the caller's.

unset(Ice_PREFIX)
unset(Ice_INCLUDE_ROOT)

get_filename_component(_ice_name "${CMAKE_CURRENT_LIST_DIR}" NAME)
get_filename_component(_ice_up1 "${CMAKE_CURRENT_LIST_DIR}" DIRECTORY)
get_filename_component(_ice_up1_name "${_ice_up1}" NAME)
get_filename_component(_ice_up2 "${_ice_up1}" DIRECTORY)
get_filename_component(_ice_up2_name "${_ice_up2}" NAME)
get_filename_component(_ice_up3 "${_ice_up2}" DIRECTORY)
get_filename_component(_ice_up3_name "${_ice_up3}" NAME)

if(_ice_name STREQUAL "cmake")
  set(_ice_prefix "${_ice_up1}")
elseif(_ice_up1_name STREQUAL "cmake" AND _ice_up2_name MATCHES "^lib")
  set(_ice_prefix "${_ice_up3}")
elseif(_ice_up1_name STREQUAL "cmake" AND _ice_up3_name MATCHES "^lib")
  get_filename_component(_ice_prefix "${_ice_up3}" DIRECTORY)
else()
  set(_ice_prefix "")
endif()

if(_ice_prefix)
  # The NuGet package nests everything under build/native.
  foreach(_ice_subdir "build/native/include" "include")
    if(EXISTS "${_ice_prefix}/${_ice_subdir}/Ice/Ice.h")
      set(Ice_PREFIX "${_ice_prefix}")
      set(Ice_INCLUDE_ROOT "${_ice_prefix}/${_ice_subdir}")
      break()
    endif()
  endforeach()
endif()

unset(_ice_name)
unset(_ice_up1)
unset(_ice_up1_name)
unset(_ice_up2)
unset(_ice_up2_name)
unset(_ice_up3)
unset(_ice_up3_name)
unset(_ice_prefix)
unset(_ice_subdir)
