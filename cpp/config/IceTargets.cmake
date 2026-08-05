# Copyright (c) ZeroC, Inc.

cmake_policy(VERSION 3.21)

if(WIN32 AND NOT DEFINED Ice_WIN32_PLATFORM)
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(Ice_WIN32_PLATFORM "x64" CACHE STRING "Use x64 Ice library")
  else()
    set(Ice_WIN32_PLATFORM "Win32" CACHE STRING "Use Win32 Ice library")
  endif()
endif()

# REQUIRED throughout: this file only runs once find_package accepted the config file, and the
# package installs as a unit, so we know exactly what ships next to it and assume it is available.
# A missing piece is a broken installation and fails loudly right here. QUIET is no reason to
# tolerate that - it only disables messages when the package cannot be found, and a package whose
# config file was found but whose contents are gone is broken, not absent.
find_path(Ice_INCLUDE_DIR NAMES Ice/Ice.h
  HINTS ${Ice_INCLUDE_ROOT} ${Ice_PREFIX} ${Ice_PREFIX}/build/native
  PATH_SUFFIXES include DOC "Directory containing Ice header files"
  NO_DEFAULT_PATH REQUIRED)

find_program(Ice_SLICE2CPP_EXECUTABLE slice2cpp
  HINTS ${Ice_PREFIX}
  PATH_SUFFIXES bin tools
  DOC "Path to the slice2cpp compiler"
  NO_DEFAULT_PATH REQUIRED
)

# The guard lets two subprojects in one directory scope each call find_package(Ice) without the
# second failing on a duplicate target.
if(NOT TARGET Ice::slice2cpp)
  add_executable(Ice::slice2cpp IMPORTED)
  set_target_properties(Ice::slice2cpp PROPERTIES
    IMPORTED_LOCATION "${Ice_SLICE2CPP_EXECUTABLE}"
  )
endif()

find_path(Ice_SLICE_DIR
  NAMES Ice/Identity.ice
  HINTS ${Ice_PREFIX}
  PATH_SUFFIXES slice share/ice/slice
  DOC "Path to the Ice Slice files directory"
  NO_DEFAULT_PATH REQUIRED)

# Imported targets for the executables in the NuGet package's native/bin directory.
if(WIN32)
  function(add_ice_executable name)
    find_program(Ice_${name}_EXE_RELEASE ${name}${CMAKE_EXECUTABLE_SUFFIX}
      HINTS "${Ice_PREFIX}/build/native/bin/${Ice_WIN32_PLATFORM}/Release"
      NO_DEFAULT_PATH
    )

    find_program(Ice_${name}_EXE_DEBUG ${name}${CMAKE_EXECUTABLE_SUFFIX}
      HINTS "${Ice_PREFIX}/build/native/bin/${Ice_WIN32_PLATFORM}/Debug"
      NO_DEFAULT_PATH
    )

    # Better undefined than a target with no IMPORTED_LOCATION, which fails far less obviously.
    if(NOT Ice_${name}_EXE_RELEASE AND NOT Ice_${name}_EXE_DEBUG)
      return()
    endif()

    if(TARGET Ice::${name}_EXE)
      return()
    endif()

    add_executable(Ice::${name}_EXE IMPORTED)

    if(Ice_${name}_EXE_RELEASE)
      set_property(TARGET Ice::${name}_EXE PROPERTY
        IMPORTED_LOCATION_RELEASE "${Ice_${name}_EXE_RELEASE}")
    endif()

    if(Ice_${name}_EXE_DEBUG)
      set_property(TARGET Ice::${name}_EXE PROPERTY
        IMPORTED_LOCATION_DEBUG "${Ice_${name}_EXE_DEBUG}")
    endif()

    # Set the appropriate location based on build type
    if(Ice_${name}_EXE_RELEASE OR Ice_${name}_EXE_DEBUG)
      set_property(TARGET Ice::${name}_EXE PROPERTY
        IMPORTED_LOCATION "$<IF:$<CONFIG:Debug>,${Ice_${name}_EXE_DEBUG},${Ice_${name}_EXE_RELEASE}>")
    endif()
  endfunction()

  add_ice_executable(icebox)
endif()

# Adds an Ice::<component> target with the specified link libraries
function(add_ice_target component)
  set(link_libraries ${ARGN})

  if(TARGET Ice::${component})
    return()
  endif()

  # Ice ships static archives alongside the shared libraries, so match the type actually found. A
  # static Ice still needs its own system dependencies (bzip2, OpenSSL, ...), which we do not list.
  set(library_type SHARED)
  if(NOT WIN32 AND Ice_${component}_LIBRARY_RELEASE MATCHES "\\${CMAKE_STATIC_LIBRARY_SUFFIX}$")
    set(library_type STATIC)
  endif()

  add_library(Ice::${component} ${library_type} IMPORTED)
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
    set_target_properties(Ice::${component} PROPERTIES
      IMPORTED_LOCATION "${Ice_${component}_LIBRARY_RELEASE}"
    )
  endif()
endfunction()

# Finds an Ice component and, when present, defines an Ice::<component> imported target linking the
# given libraries. Every argument after <component> is treated as a link library.
function(add_ice_library component)

  # If CMAKE_LIBRARY_ARCHITECTURE is set we check /lib/<arch> and /lib<arch> separately, to avoid a
  # single find_library with both HINTS and PATH_SUFFIXES giving false positives on multi-arch.
  if(WIN32)
    # Find Release and Debug libraries on Windows inside the NuGet package
    find_library(Ice_${component}_IMPLIB_RELEASE
      NAMES ${component}${Ice_SO_VERSION}
      HINTS "${Ice_PREFIX}/build/native/lib/${Ice_WIN32_PLATFORM}/Release"
      NO_DEFAULT_PATH
    )

    find_library(Ice_${component}_IMPLIB_DEBUG
      NAMES ${component}d ${component}${Ice_SO_VERSION}d
      HINTS "${Ice_PREFIX}/build/native/lib/${Ice_WIN32_PLATFORM}/Debug"
      NO_DEFAULT_PATH
    )

    find_file(Ice_${component}_LIBRARY_RELEASE
      NAMES ${component}${Ice_SO_VERSION}.dll
      HINTS "${Ice_PREFIX}/build/native/bin/${Ice_WIN32_PLATFORM}/Release"
      NO_DEFAULT_PATH
    )

    find_file(Ice_${component}_LIBRARY_DEBUG
      NAMES ${component}${Ice_SO_VERSION}d.dll
      HINTS "${Ice_PREFIX}/build/native/bin/${Ice_WIN32_PLATFORM}/Debug"
      NO_DEFAULT_PATH
    )
  elseif(DEFINED CMAKE_LIBRARY_ARCHITECTURE AND EXISTS "${Ice_PREFIX}/lib/${CMAKE_LIBRARY_ARCHITECTURE}")
    # Find the library in the <prefix>/lib/<arch> directory (e.g. Debian/Ubuntu)
    find_library(
      Ice_${component}_LIBRARY_RELEASE
      NAMES ${component}
      HINTS  "${Ice_PREFIX}/lib/${CMAKE_LIBRARY_ARCHITECTURE}"
      NO_DEFAULT_PATH
    )
  elseif(DEFINED CMAKE_LIBRARY_ARCHITECTURE AND EXISTS "${Ice_PREFIX}/lib${CMAKE_LIBRARY_ARCHITECTURE}")
    # Find the library  in the <prefix>/lib<arch>/ directory (e.g. RHEL)
    find_library(
      Ice_${component}_LIBRARY_RELEASE
      NAMES ${component}
      HINTS  "${Ice_PREFIX}/lib${CMAKE_LIBRARY_ARCHITECTURE}"
      NO_DEFAULT_PATH
    )
  else()
    # Find the library in the <prefix>/lib/ directory (e.g. macOS, other Linux)
    find_library(
      Ice_${component}_LIBRARY_RELEASE
      NAMES ${component}
      HINTS  "${Ice_PREFIX}/lib"
      NO_DEFAULT_PATH
    )
  endif()

  # Merges the per-config results into Ice_<component>_LIBRARY for the check below. The variable is
  # function-local; consumers see only the Ice_<component>_LIBRARY_<CONFIG> cache entries.
  include(SelectLibraryConfigurations)
  select_library_configurations(Ice_${component})

  # Components vary by platform (IceBT ships on Linux only), so an absent component library is
  # normal; everything a present component links is guaranteed by the package.
  if(NOT Ice_${component}_LIBRARY)
    return()
  endif()

  # find_package_handle_standard_args reads this to decide the component was found.
  set(Ice_${component}_FOUND TRUE PARENT_SCOPE)
  add_ice_target(${component} ${ARGN})

endfunction()

include(CMakeFindDependencyMacro)

# find_dependency forwards our QUIET/REQUIRED and reports Ice as not found rather than aborting.
find_dependency(Threads)

add_ice_library(Ice Threads::Threads)
if(WIN32)
  # Bzip2 is included in the Ice NuGet package and is a runtime dependency of Ice.
  # This property can be used to copy the correct DLLs to the target directory at build time.
  set_property(TARGET Ice::Ice PROPERTY ICE_RUNTIME_DLLS
    "$<$<CONFIG:Debug>:${Ice_PREFIX}/build/native/bin/${Ice_WIN32_PLATFORM}/Debug/bzip2d.dll>"
    "$<$<CONFIG:Release>:${Ice_PREFIX}/build/native/bin/${Ice_WIN32_PLATFORM}/Release/bzip2.dll>"
  )
endif()
add_ice_library(DataStorm Ice::Ice)
add_ice_library(Glacier2 Ice::Ice)
add_ice_library(IceBox Ice::Ice)
add_ice_library(IceDiscovery Ice::Ice)
add_ice_library(IceGrid Ice::Ice Ice::Glacier2)
add_ice_library(IceLocatorDiscovery Ice::Ice)
add_ice_library(IceStorm Ice::Ice)
add_ice_library(IceBT Ice::Ice)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Ice HANDLE_COMPONENTS HANDLE_VERSION_RANGE CONFIG_MODE)
