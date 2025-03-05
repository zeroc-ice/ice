# Copyright (c) ZeroC, Inc.

if(WIN32 AND NOT DEFINED Ice_WIN32_PLATFORM)
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(Ice_WIN32_PLATFORM "x64" CACHE PATH "Use x64 Ice library")
  else()
    set(Ice_WIN32_PLATFORM "Win32" CACHE PATH "Use Win32 Ice library")
  endif()
endif()

find_path(Ice_INCLUDE_DIR NAMES Ice/Ice.h
  HINTS ${PACKAGE_PREFIX_DIR} ${PACKAGE_PREFIX_DIR}/build/native
  PATH_SUFFIXES include DOC "Directory containing Ice header files"
  NO_DEFAULT_PATH
  REQUIRED)

# Read Ice version variables from Ice/Config.h
if(NOT DEFINED Ice_VERSION)
  file(STRINGS "${Ice_INCLUDE_DIR}/Ice/Config.h" _ice_config_h_content REGEX "#define ICE_([A-Z]+)_VERSION ")

  if("${_ice_config_h_content}" MATCHES "#define ICE_STRING_VERSION \"([^\"]+)\"")
    set(Ice_VERSION "${CMAKE_MATCH_1}" CACHE STRING "Ice version")
  endif()

  if("${_ice_config_h_content}" MATCHES "#define ICE_SO_VERSION \"([^\"]+)\"")
    set(Ice_SO_VERSION "${CMAKE_MATCH_1}" CACHE STRING "Ice SO version")
  endif()
  unset(_ice_config_h_content)
endif()

find_program(Ice_SLICE2CPP_EXECUTABLE slice2cpp
  HINTS ${PACKAGE_PREFIX_DIR}
  PATH_SUFFIXES bin tools
  DOC "Path to the slice2cpp compiler"
  NO_DEFAULT_PATH
  REQUIRED
)

# Add an imported target for slice2cpp
add_executable(Ice::slice2cpp IMPORTED)
set_target_properties(Ice::slice2cpp PROPERTIES
  IMPORTED_LOCATION "${Ice_SLICE2CPP_EXECUTABLE}"
)

find_path(Ice_SLICE_DIR
  NAMES Ice/Identity.ice
  HINTS ${PACKAGE_PREFIX_DIR}
  PATH_SUFFIXES slice share/ice/slice
  DOC "Path to the Ice Slice files directory"
  NO_DEFAULT_PATH
  REQUIRED)

# Adds an Ice:<component> target with the specified link libraries
function(add_ice_target component link_libraries)
  add_library(Ice::${component} SHARED IMPORTED)
  set_target_properties(Ice::${component} PROPERTIES
    INTERFACE_COMPILE_FEATURES "cxx_std_17"
    INTERFACE_INCLUDE_DIRECTORIES "${Ice_INCLUDE_DIR}"
    INTERFACE_LINK_LIBRARIES "${link_libraries}"
  )

  if(WIN32)
    if(Ice_${component}_LIBRARY_RELEASE)
      set_target_properties(Ice::${component} PROPERTIES
        IMPORTED_CONFIGURATIONS RELEASE
        IMPORTED_IMPLIB_RELEASE "${Ice_${component}_IMPLIB_RELEASE}"
        IMPORTED_LOCATION_RELEASE "${Ice_${component}_LIBRARY_RELEASE}"
      )
    endif()

    if(Ice_${component}_LIBRARY_DEBUG)
      set_target_properties(Ice::${component} PROPERTIES
        IMPORTED_CONFIGURATIONS DEBUG
        IMPORTED_IMPLIB_DEBUG "${Ice_${component}_IMPLIB_DEBUG}"
        IMPORTED_LOCATION_DEBUG "${Ice_${component}_LIBRARY_DEBUG}"
      )
    endif()

  else()
    if(Ice_${component}_LIBRARY)
      set_target_properties(Ice::${component} PROPERTIES
        IMPORTED_LOCATION "${Ice_${component}_LIBRARY}"
      )
    endif()
  endif()

endfunction()

function(add_ice_library component link_libraries)

  if(WIN32)
    # Find Release and Debug libraries on Windows
    find_library(Ice_${component}_IMPLIB_RELEASE
      NAMES ${component}${Ice_SO_VERSION}
      HINTS "${PACKAGE_PREFIX_DIR}/build/native/lib/${Ice_WIN32_PLATFORM}/Release"
      NO_DEFAULT_PATH
    )

    message(STATUS "Ice_${component}_IMPLIB_RELEASE: ${Ice_${component}_IMPLIB_RELEASE}")
    message(STATUS "${PACKAGE_PREFIX_DIR}/build/native/lib/${Ice_WIN32_PLATFORM}/Release")

    find_library(Ice_${component}_IMPLIB_DEBUG
      NAMES ${component}d ${component}${Ice_SO_VERSION}d
      HINTS "${PACKAGE_PREFIX_DIR}/build/native/lib/${Ice_WIN32_PLATFORM}/Debug"
      NO_DEFAULT_PATH
    )

    find_file(Ice_${component}_LIBRARY_RELEASE
      NAMES ${component}${Ice_SO_VERSION}.dll
      HINTS "${PACKAGE_PREFIX_DIR}/build/native/bin/${Ice_WIN32_PLATFORM}/Release"
      NO_DEFAULT_PATH
    )

    find_file(Ice_${component}_LIBRARY_DEBUG
      NAMES ${component}${Ice_SO_VERSION}d.dll
      HINTS "${PACKAGE_PREFIX_DIR}/build/native/bin/${Ice_WIN32_PLATFORM}/Debug"
      NO_DEFAULT_PATH
    )
  else()
    # Linux and macOS only have one library configuration
    find_library(
      Ice_${component}_LIBRARY_RELEASE
      NAMES ${component}
      HINTS "${PACKAGE_PREFIX_DIR}/lib"
      PATH_SUFFIXES "${CMAKE_LIBRARY_ARCHITECTURE}"
      NO_DEFAULT_PATH
    )
  endif()

  # Select the appropriate library configuration based on platform and build type
  include(SelectLibraryConfigurations)
  select_library_configurations(Ice_${component})

  if(Ice_${component}_LIBRARY)
    # Set the Ice_<component>_FOUND variable to TRUE so that find_package_handle_standard_args
    # will consider the component found
    set(Ice_${component}_FOUND TRUE PARENT_SCOPE)
    add_ice_target(${component} "${link_libraries}")
  endif()

endfunction()

include(CMakeFindDependencyMacro)
find_package(Threads REQUIRED QUIET)

add_ice_library(Ice Threads::Threads)
add_ice_library(DataStorm Ice::Ice)
add_ice_library(Glacier2 Ice::Ice)
add_ice_library(IceBox Ice::Ice)
add_ice_library(IceDiscovery Ice::Ice)
add_ice_library(IceGrid Ice::Ice Ice::Glacier2)
add_ice_library(IceLocatorDiscovery Ice::Ice)
add_ice_library(IceStorm Ice::Ice)
add_ice_library(IceBT Ice::Ice)

include(FindPackageHandleStandardArgs)
set(${CMAKE_FIND_PACKAGE_NAME}_CONFIG "${CMAKE_CURRENT_LIST_FILE}")
find_package_handle_standard_args(Ice HANDLE_COMPONENTS CONFIG_MODE)
