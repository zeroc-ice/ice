# Copyright (c) ZeroC, Inc.

# Reports this installation's version to find_package. Without this file every
# find_package(Ice <version>) fails with "version: unknown".
#
# CMake's own way to produce this file is write_basic_package_version_file(), but that runs inside a
# CMake build and the Ice C++ build has none, so the file is maintained by hand. Plain requests follow
# the COMPATIBILITY SameMinorVersion policy; version ranges are honored on both endpoints, where
# SameMinorVersion would reject an upper endpoint in a later x.y.

include("${CMAKE_CURRENT_LIST_DIR}/IcePrefix.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/IceVersion.cmake")

# Report the full version, pre-release suffix included, so Ice_VERSION matches what Ice reports
# everywhere else. CMake's version comparison ignores the suffix and still derives
# Ice_VERSION_MAJOR/MINOR/PATCH from the numeric part.
set(PACKAGE_VERSION "${Ice_VERSION}")

# An installation whose version cannot be read is unusable whatever was requested, so it is
# UNSUITABLE rather than merely incompatible: CMake then passes over it even for an unversioned
# request, as its own generated version files do for a bitness mismatch. The reason goes in
# PACKAGE_VERSION, which CMake prints against the rejected candidate, likewise following those files.
if(NOT PACKAGE_VERSION)
  set(PACKAGE_VERSION "unknown - broken installation, cannot read the Ice version from Ice/Config.h")
  set(PACKAGE_VERSION_UNSUITABLE TRUE)
endif()

if(PACKAGE_VERSION MATCHES "^([0-9]+)\\.([0-9]+)")
  set(_ice_version_major "${CMAKE_MATCH_1}")
  set(_ice_version_minor "${CMAKE_MATCH_2}")
else()
  # Fail closed on a malformed version rather than compare against stale CMAKE_MATCH values.
  set(_ice_version_major "")
  set(_ice_version_minor "")
endif()

set(PACKAGE_VERSION_COMPATIBLE FALSE)
set(PACKAGE_VERSION_EXACT FALSE)

# A request is compatible when it names this x.y and is not newer than this installation. Ice
# treats every x.y as a major release: 3.8 and 3.9 are not binary compatible and install side by
# side (Ice-3.8, Ice-3.9); only patch releases within an x.y line are compatible. With 3.9.2
# installed, requests for 3.9, 3.9.0 and 3.9.2 match; 3.9.5, 3.8, 3.7, 4.0 and a bare 3 do not.
if(PACKAGE_VERSION)
  if(PACKAGE_FIND_VERSION_RANGE)
    # Same rule on the range's lower endpoint, plus this version must not exceed the upper one.
    if(PACKAGE_FIND_VERSION_MIN_MAJOR STREQUAL _ice_version_major AND
       PACKAGE_FIND_VERSION_MIN_MINOR STREQUAL _ice_version_minor AND
       NOT PACKAGE_VERSION VERSION_LESS PACKAGE_FIND_VERSION_MIN)
      if(PACKAGE_FIND_VERSION_RANGE_MAX STREQUAL "INCLUDE" AND
         NOT PACKAGE_VERSION VERSION_GREATER PACKAGE_FIND_VERSION_MAX)
        set(PACKAGE_VERSION_COMPATIBLE TRUE)
      elseif(PACKAGE_FIND_VERSION_RANGE_MAX STREQUAL "EXCLUDE" AND
             PACKAGE_VERSION VERSION_LESS PACKAGE_FIND_VERSION_MAX)
        set(PACKAGE_VERSION_COMPATIBLE TRUE)
      endif()
    endif()
  else()
    if(PACKAGE_VERSION VERSION_LESS PACKAGE_FIND_VERSION)
      # Older than asked for.
      set(PACKAGE_VERSION_COMPATIBLE FALSE)
    elseif(PACKAGE_FIND_VERSION_MAJOR STREQUAL _ice_version_major AND
           PACKAGE_FIND_VERSION_MINOR STREQUAL _ice_version_minor)
      set(PACKAGE_VERSION_COMPATIBLE TRUE)
    endif()

    # STREQUAL, as in CMake's own generated version files: a pre-release must not satisfy EXACT
    # for the release it precedes, so 3.9.0-alpha.0 does not answer 3.9.0 EXACT. Plain requests
    # still accept it. EXACT never combines with a range; find_package refuses that itself.
    if(PACKAGE_FIND_VERSION STREQUAL PACKAGE_VERSION)
      set(PACKAGE_VERSION_EXACT TRUE)
    endif()
  endif()
endif()

unset(_ice_version_major)
unset(_ice_version_minor)
