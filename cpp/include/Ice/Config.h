// Copyright (c) ZeroC, Inc.

#ifndef ICE_CONFIG_H
#define ICE_CONFIG_H

// Compiler extensions to export and import symbols: see the documentation for Visual Studio, Clang and GCC.
#if defined(_MSC_VER)
#    define ICE_DECLSPEC_EXPORT __declspec(dllexport)
#    define ICE_DECLSPEC_IMPORT __declspec(dllimport)
//  With Visual Studio, we can import/export member functions without importing/exporting the whole class.
#    define ICE_MEMBER_IMPORT_EXPORT
#elif defined(__GNUC__) || defined(__clang__)
#    define ICE_DECLSPEC_EXPORT __attribute__((visibility("default")))
#    define ICE_DECLSPEC_IMPORT __attribute__((visibility("default")))
#else
#    define ICE_DECLSPEC_EXPORT /**/
#    define ICE_DECLSPEC_IMPORT /**/
#endif

#ifdef ICE_MEMBER_IMPORT_EXPORT
#    define ICE_CLASS(API) /**/
#    define ICE_MEMBER(API) API
#else
#    define ICE_CLASS(API) API
#    define ICE_MEMBER(API) /**/
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

// The Ice version.
// NOLINTBEGIN(modernize-macro-to-enum)
#define ICE_STRING_VERSION "3.8.0-alpha.0" // "A.B.C", with A=major, B=minor, C=patch
#define ICE_INT_VERSION 30850              // AABBCC, with AA=major, BB=minor, CC=patch
#define ICE_SO_VERSION "38a0"              // "ABC", with A=major, B=minor, C=patch
// NOLINTEND(modernize-macro-to-enum)

#if !defined(ICE_BUILDING_ICE) && defined(ICE_API_EXPORTS)
#    define ICE_BUILDING_ICE
#endif

#if defined(_MSC_VER) && !defined(ICE_BUILDING_SLICE_COMPILERS) // Not using the IceUtil static build
#    if !defined(_DLL) || !defined(_MT)
#        error "Ice C++ applications on Windows require /MD or /MDd."
#    endif
#    if defined(_DEBUG)
#        define ICE_LIBNAME(NAME) NAME ICE_SO_VERSION "D.lib"
#    else
#        define ICE_LIBNAME(NAME) NAME ICE_SO_VERSION ".lib"
#    endif
#    if !defined(ICE_BUILDING_ICE)
#        pragma comment(lib, ICE_LIBNAME("Ice")) // Automatically link with Ice[D].lib
#    endif
#endif

#ifdef __APPLE__
#    include <TargetConditionals.h>
#endif

#endif
