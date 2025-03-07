# Copyright (c) ZeroC, Inc.

if(WIN32)
    # On Windows Ice ships as a NuGet package, so we need to find the package prefix directory.
    # This config file is located at <ice_nuget_package>/cmake/IceConfig.cmake
    get_filename_component(Ice_PREFIX "${CMAKE_CURRENT_LIST_DIR}/../" ABSOLUTE)
else()
    # This file is located at either:
    # -  <ice_install_prefix>/<lib>/cmake/Ice/IceConfig.cmake
    # -  <ice_install_prefix>/<lib>/<arch>/cmake/Ice/IceConfig.cmake
    get_filename_component(Ice_PREFIX "${CMAKE_CURRENT_LIST_DIR}/../../../" ABSOLUTE)
    if(DEFINED CMAKE_LIBRARY_ARCHITECTURE)
        get_filename_component(Ice_PREFIX "${Ice_PREFIX}/.." ABSOLUTE)
    endif()
endif()

include("${CMAKE_CURRENT_LIST_DIR}/IceTargets.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/slice2cpp.cmake")
