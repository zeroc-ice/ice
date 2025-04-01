// Copyright (c) ZeroC, Inc.

#ifndef ICE_CONFIG_H
#define ICE_CONFIG_H

// Compiler extensions to export and import symbols: see the documentation for Visual Studio, Clang and GCC.
#if defined(_MSC_VER)
#    define ICE_DECLSPEC_EXPORT __declspec(dllexport)
#    define ICE_DECLSPEC_IMPORT __declspec(dllimport)
#elif defined(__GNUC__) || defined(__clang__)
#    define ICE_DECLSPEC_EXPORT [[gnu::visibility("default")]]
#    define ICE_DECLSPEC_IMPORT [[gnu::visibility("default")]]
#else
#    define ICE_DECLSPEC_EXPORT /**/
#    define ICE_DECLSPEC_IMPORT /**/
#endif

#ifndef ICE_API
#    if defined(ICE_BUILDING_SLICE_COMPILERS) // Only defined for Windows builds.
#        define ICE_API                       /**/
#    elif defined(ICE_API_EXPORTS)
#        define ICE_API ICE_DECLSPEC_EXPORT
#    else
#        define ICE_API ICE_DECLSPEC_IMPORT
#    endif
#endif

// For friend declarations on Windows. Not compatible with the standard c++ attribute syntax.
#ifndef ICE_API_FRIEND
#    if defined(_MSC_VER)
#        define ICE_API_FRIEND ICE_API
#    else
#        define ICE_API_FRIEND /**/
#    endif
#endif

// The Ice version.
#define ICE_STRING_VERSION "3.8.0-alpha.0" // "A.B.C", with A=major, B=minor, C=patch
#define ICE_INT_VERSION 30850              // AABBCC, with AA=major, BB=minor, CC=patch
#define ICE_SO_VERSION "38a0"              // "ABC", with A=major, B=minor, C=patch

#if defined(_MSC_VER) && !defined(ICE_BUILDING_SLICE_COMPILERS) // Not using the IceUtil static build
#    if !defined(_DLL) || !defined(_MT)
#        error "Ice C++ applications on Windows require /MD or /MDd."
#    endif
#    if defined(_DEBUG)
#        define ICE_LIBNAME(NAME) NAME ICE_SO_VERSION "D.lib"
#    else
#        define ICE_LIBNAME(NAME) NAME ICE_SO_VERSION ".lib"
#    endif
#    if !defined(ICE_API_EXPORTS) && !defined(ICE_DISABLE_PRAGMA_COMMENT)
#        pragma comment(lib, ICE_LIBNAME("Ice")) // Automatically link with Ice[D].lib
#    endif
#endif

#ifdef __APPLE__
#    include <TargetConditionals.h>
#endif

#endif
