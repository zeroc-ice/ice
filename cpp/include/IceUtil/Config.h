//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_UTIL_CONFIG_H
#define ICE_UTIL_CONFIG_H

// Use the system headers as preferred way to detect endianness and fallback to architecture based checks.

#include <stdlib.h>

#ifdef _MSC_VER

#    ifdef _WIN64
#        define ICE_64
#    else
#        define ICE_32
#    endif

#else

// Use system headers as preferred way to detect 32 or 64 bit mode and fallback to architecture based checks.
#    include <stdint.h>

#    if defined(__WORDSIZE) && (__WORDSIZE == 64)
#        define ICE_64
#    elif defined(__WORDSIZE) && (__WORDSIZE == 32)
#        define ICE_32
#    elif defined(__sun) && (defined(__sparcv9) || defined(__x86_64)) || defined(__linux__) && defined(__x86_64) ||    \
        defined(__APPLE__) && defined(__x86_64) || defined(__hppa) && defined(__LP64__) ||                             \
        defined(_ARCH_COM) && defined(__64BIT__) || defined(__alpha__) || defined(_WIN64)
#        define ICE_64
#    else
#        define ICE_32
#    endif
#endif

#if defined(_MSVC_LANG)
#    define ICE_CPLUSPLUS _MSVC_LANG
#else
#    define ICE_CPLUSPLUS __cplusplus
#endif

// Compiler extensions to export and import symbols: see the documentation for Visual Studio, Clang and GCC.
#if defined(_MSC_VER)
#    define ICE_DECLSPEC_EXPORT __declspec(dllexport)
#    define ICE_DECLSPEC_IMPORT __declspec(dllimport)
//  With Visual Studio, we can import/export member functions without importing/
//  exporting the whole class
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
#    if defined(ICE_STATIC_LIBS)
#        define ICE_API /**/
#    elif defined(ICE_API_EXPORTS)
#        define ICE_API ICE_DECLSPEC_EXPORT
#    else
#        define ICE_API ICE_DECLSPEC_IMPORT
#    endif
#endif

#if defined(_MSC_VER)
#    define ICE_DEPRECATED_API(msg) __declspec(deprecated(msg))
#elif defined(__clang__)
#    if __has_extension(attribute_deprecated_with_message)
#        define ICE_DEPRECATED_API(msg) __attribute__((deprecated(msg)))
#    else
#        define ICE_DEPRECATED_API(msg) __attribute__((deprecated))
#    endif
#elif defined(__GNUC__)
#    define ICE_DEPRECATED_API(msg) __attribute__((deprecated(msg)))
#else
#    define ICE_DEPRECATED_API(msg) /**/
#endif

#if defined(__clang__) || defined(__GNUC__)
#    define ICE_MAYBE_UNUSED __attribute__((unused))
#else
#    define ICE_MAYBE_UNUSED /**/
#endif

#ifdef _WIN32
#    include <windows.h>
#    define ICE_HAS_WIN32_CONDVAR
#endif

// Some include files we need almost everywhere.
#include <cassert>
#include <iostream>
#include <sstream>
#include <exception>
#include <stdexcept>
#include <memory>
#include <future>

#ifndef _WIN32
#    include <pthread.h>
#    include <errno.h>
#    include <unistd.h>
#endif

#ifdef __APPLE__
#    include <TargetConditionals.h>
#endif

#if defined(_AIX) && defined(_LARGE_FILES)
// defines macros such as open that we want to use consistently everywhere
#    include <fcntl.h>
#endif

// The Ice version.
#define ICE_STRING_VERSION "3.8.0-alpha.0" // "A.B.C", with A=major, B=minor, C=patch
#define ICE_INT_VERSION 30850              // AABBCC, with AA=major, BB=minor, CC=patch
#define ICE_SO_VERSION "38a0"              // "ABC", with A=major, B=minor, C=patch

#if !defined(ICE_BUILDING_ICE) && defined(ICE_API_EXPORTS)
#    define ICE_BUILDING_ICE
#endif

#if defined(_MSC_VER)
#    if !defined(ICE_STATIC_LIBS) && (!defined(_DLL) || !defined(_MT))
#        error "Only multi-threaded DLL libraries can be used with Ice!"
#    endif
#    if defined(_DEBUG)
#        define ICE_LIBNAME(NAME) NAME ICE_SO_VERSION "D.lib"
#    else
#        define ICE_LIBNAME(NAME) NAME ICE_SO_VERSION ".lib"
#    endif
//  Automatically link with Ice[D].lib when using MSVC
#    if !defined(ICE_BUILDING_ICE) && !defined(ICE_BUILDING_SLICE_COMPILERS)
#        pragma comment(lib, ICE_LIBNAME("Ice"))
#    endif
#endif

namespace IceUtil
{

    //
    // By deriving from this class, other classes are made non-copyable.
    //
    class ICE_API noncopyable
    {
    protected:
        noncopyable() {}
        ~noncopyable() {} // May not be virtual! Classes without virtual
                          // operations also derive from noncopyable.

    private:
        noncopyable(const noncopyable&);
        const noncopyable& operator=(const noncopyable&);
    };

//
// Int64 typedef and ICE_INT64 macro for Int64 literal values
//
// Note that on Windows, long is always 32-bit
//
#if defined(_WIN32) && defined(_MSC_VER)
    typedef __int64 Int64;
#    define ICE_INT64(n) n##i64
#    define ICE_INT64_FORMAT "%lld"
#elif defined(ICE_64) && !defined(_WIN32)
    typedef long Int64;
#    define ICE_INT64(n) n##L
#    define ICE_INT64_FORMAT "%ld"
#else
    typedef long long Int64;
#    define ICE_INT64(n) n##LL
#    define ICE_INT64_FORMAT "%lld"
#endif

}

#endif
