
//
// Copyright (c) ZeroC, Inc. All rights reserved.
//
#pragma once

#define DATASTORM_VERSION 1, 1, 1, 0
#define DATASTORM_STRING_VERSION "1.1.1"
#define DATASTORM_SO_VERSION "11"

#if defined(_DEBUG)
#    define DATASTORM_LIBNAME(NAME) NAME DATASTORM_SO_VERSION "d"
#else
#    define DATASTORM_LIBNAME(NAME) NAME DATASTORM_SO_VERSION ""
#endif

#if defined(_WIN32)
#    define DATASTORM_DECLSPEC_EXPORT __declspec(dllexport)
#    define DATASTORM_DECLSPEC_IMPORT __declspec(dllimport)
#elif defined(__GNUC__)
#    define DATASTORM_DECLSPEC_EXPORT __attribute__((visibility("default")))
#    define DATASTORM_DECLSPEC_IMPORT __attribute__((visibility("default")))
#else
#    define DATASTORM_DECLSPEC_EXPORT /**/
#    define DATASTORM_DECLSPEC_IMPORT /**/
#endif

#ifndef DATASTORM_API
#    ifdef DATASTORM_API_EXPORTS
#        define DATASTORM_API DATASTORM_DECLSPEC_EXPORT
#    elif defined(DATASTORM_STATIC_LIBS)
#        define DATASTORM_API /**/
#    else
#        define DATASTORM_API DATASTORM_DECLSPEC_IMPORT
#    endif
#endif

#if !defined(DATASTORM_BUILDING_DATASTORM) && defined(DATASTORM_API_EXPORTS)
#    define DATASTORM_BUILDING_DATASTORM
#endif

//
// Automatically link with DataStorm0[d].lib
//
#if defined(_MSC_VER) && !defined(DATASTORM_BUILDING_DATASTORM)
#    pragma comment(lib, DATASTORM_LIBNAME("DataStorm"))
#endif
