# Copyright (c) ZeroC, Inc.

if(WIN32)
    # On Windows Ice ships was a NuGet package, so we need to find the package prefix directory.
    # This config file is located at <ice_nuget_package>/build/native/lib/cmake/ice/IceConfig.cmake
    get_filename_component(PACKAGE_PREFIX_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../../../" ABSOLUTE)
else()
    # This file is located at <ice_install_prefix>/lib/cmake/Ice/IceConfig.cmake
    get_filename_component(PACKAGE_PREFIX_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../" ABSOLUTE)
endif()

include("${CMAKE_CURRENT_LIST_DIR}/IceTargets.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/slice2cpp.cmake")
