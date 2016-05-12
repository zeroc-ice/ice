// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_UTIL_CONFIG_H
#define ICE_UTIL_CONFIG_H

//
// Endianness
//
// Most CPUs support only one endianness, with the notable exceptions
// of Itanium (IA64) and MIPS.
//
#ifdef __GLIBC__
# include <endian.h>
#endif

#if defined(__i386)     || defined(_M_IX86) || defined(__x86_64)  || \
    defined(_M_X64)     || defined(_M_IA64) || defined(__alpha__) || \
    defined(__ARMEL__) || defined(_M_ARM_FP) || defined(__arm64) || \
    defined(__MIPSEL__) || (defined(__BYTE_ORDER) && (__BYTE_ORDER == __LITTLE_ENDIAN))
#   define ICE_LITTLE_ENDIAN
#elif defined(__sparc) || defined(__sparc__) || defined(__hppa)      || \
      defined(__ppc__) || defined(__powerpc) || defined(_ARCH_COM) || \
      defined(__MIPSEB__) || (defined(__BYTE_ORDER) && (__BYTE_ORDER == __BIG_ENDIAN))
#   define ICE_BIG_ENDIAN
#else
#   error "Unknown architecture"
#endif

//
// 32 or 64 bit mode?
//
#if defined(__sun) && (defined(__sparcv9) || defined(__x86_64))    || \
      defined(__linux) && defined(__x86_64)                        || \
      defined(__APPLE__) && defined(__x86_64)                      || \
      defined(__hppa) && defined(__LP64__)                         || \
      defined(_ARCH_COM) && defined(__64BIT__)                     || \
      defined(__alpha__)                                           || \
      defined(_WIN64)
#   define ICE_64
#else
#   define ICE_32
#endif

//
// Check for C++ 11 support
//
// For GCC, we recognize --std=c++0x only for GCC version 4.5 and greater,
// as C++11 support in prior releases was too limited.
//
#if (__cplusplus >= 201103) || \
    ((defined(__GNUC__) && defined(__GXX_EXPERIMENTAL_CXX0X__) && ((__GNUC__* 100) + __GNUC_MINOR__) >= 405)) || \
    (defined(_MSC_VER) && (_MSC_VER >= 1600))
#   define ICE_CPP11_COMPILER
#endif

//
// Ensure the C++ compiler supports C++11 when using the C++11 mapping
//
#if defined(ICE_CPP11_MAPPING) && !defined(ICE_CPP11_COMPILER)
#   error "you need a C++11 capable compiler to use the C++11 mapping"
#endif


#if defined(ICE_CPP11_COMPILER) && (!defined(_MSC_VER) || (_MSC_VER >= 1900))
#   define ICE_NOEXCEPT noexcept
#   define ICE_NOEXCEPT_FALSE noexcept(false)
#else
#   define ICE_NOEXCEPT throw()
#   define ICE_NOEXCEPT_FALSE /**/
#endif

//
// Visual Studio 2015 or later
//
#if defined(_MSC_VER) && (_MSC_VER >= 1900)

//
// Check if building for WinRT
//
#   include <winapifamily.h>
#   if defined(WINAPI_FAMILY) && (WINAPI_FAMILY == WINAPI_FAMILY_APP)
#      define ICE_OS_WINRT
#      define ICE_STATIC_LIBS
#   endif

#endif

//
// Support for thread-safe function local static initialization
// (a.k.a. "magic statics")
//
#if defined(__GNUC__) || defined(__clang__) || (defined(_MSC_VER) && (_MSC_VER >= 1900))
#   define ICE_HAS_THREAD_SAFE_LOCAL_STATIC
#endif

//
// Compiler extensions to export and import symbols: see the documentation
// for Visual Studio, Solaris Studio and GCC.
//
#if defined(_WIN32)
#   define ICE_DECLSPEC_EXPORT __declspec(dllexport)
#   define ICE_DECLSPEC_IMPORT __declspec(dllimport)
//
//  ICE_HAS_DECLSPEC_IMPORT_EXPORT defined only for compilers with distinct
//  declspec for IMPORT and EXPORT
#   define ICE_HAS_DECLSPEC_IMPORT_EXPORT
#elif defined(__GNUC__)
#   define ICE_DECLSPEC_EXPORT __attribute__((visibility ("default")))
#   define ICE_DECLSPEC_IMPORT __attribute__((visibility ("default")))
#elif defined(__SUNPRO_CC)
#   define ICE_DECLSPEC_EXPORT __global
#   define ICE_DECLSPEC_IMPORT /**/
#else
#   define ICE_DECLSPEC_EXPORT /**/
#   define ICE_DECLSPEC_IMPORT /**/
#endif

//
// Let's use these extensions with Ice:
//
#if defined(ICE_API_EXPORTS)
#   define ICE_API ICE_DECLSPEC_EXPORT
#elif defined(ICE_STATIC_LIBS)
#   define ICE_API /**/
#else
#   define ICE_API ICE_DECLSPEC_IMPORT
#endif


#if defined(_MSC_VER)
#   define ICE_DEPRECATED_API(msg) __declspec(deprecated(msg))
#elif defined(__clang__)
#   if __has_extension(attribute_deprecated_with_message)
#       define ICE_DEPRECATED_API(msg) __attribute__((deprecated(msg)))
#   else
#       define ICE_DEPRECATED_API(msg) __attribute__((deprecated))
#   endif
#elif defined(__GNUC__)
#   if (__GNUC__ > 4 || (__GNUC__ == 4 &&  __GNUC_MINOR__ >= 5))
// The message option was introduced in GCC 4.5
#      define ICE_DEPRECATED_API(msg) __attribute__((deprecated(msg)))
#   else
#      define ICE_DEPRECATED_API(msg) __attribute__((deprecated))
#   endif
#else
#   define ICE_DEPRECATED_API(msg) /**/
#endif

#ifdef _WIN32
#   include <windows.h>

#   if defined(_WIN32_WINNT) && (_WIN32_WINNT >= 0x600)
//
// Windows provides native condition variables on Vista and later
//
#      ifndef ICE_HAS_WIN32_CONDVAR
#          define ICE_HAS_WIN32_CONDVAR
#      endif
#   endif
#endif

//
// Some include files we need almost everywhere.
//
#include <cassert>
#include <iostream>
#include <sstream>

#ifndef _WIN32
#   include <pthread.h>
#   include <errno.h>
#endif

#ifdef __APPLE__
#   include <TargetConditionals.h>
#endif

#if defined(_AIX) && defined(_LARGE_FILES)
    // defines macros such as open that we want to use consistently everywhere
#   include <fcntl.h>
#endif

#ifdef __IBMCPP__
    // TODO: better fix for this warning
#   pragma report(disable, "1540-0198") // private inheritance without private keyword
#endif

//
// The Ice version.
//
#define ICE_STRING_VERSION "3.7a0" // "A.B.C", with A=major, B=minor, C=patch
#define ICE_INT_VERSION 30751      // AABBCC, with AA=major, BB=minor, CC=patch
#define ICE_SO_VERSION "37a0"      // "ABC", with A=major, B=minor, C=patch

#if !defined(ICE_BUILDING_ICE) && defined(ICE_API_EXPORTS)
#   define ICE_BUILDING_ICE
#endif

#if defined(_MSC_VER)
#   if !defined(ICE_STATIC_LIBS) && (!defined(_DLL) || !defined(_MT))
#       error "Only multi-threaded DLL libraries can be used with Ice!"
#   endif

#   ifdef ICE_CPP11_MAPPING
#      if defined(_DEBUG)
#         if defined(ICE_OS_WINRT)
#            define ICE_LIBNAME(NAME) NAME ICE_SO_VERSION "uwp++11D.lib"
#         else
#            define ICE_LIBNAME(NAME) NAME ICE_SO_VERSION "++11D.lib"
#         endif
#      else
#         if defined(ICE_OS_WINRT)
#            define ICE_LIBNAME(NAME) NAME ICE_SO_VERSION "uwp++11.lib"
#         else
#            define ICE_LIBNAME(NAME) NAME ICE_SO_VERSION "++11.lib"
#         endif
#      endif
#   else
#      if defined(_DEBUG)
#         if defined(ICE_OS_WINRT)
#            define ICE_LIBNAME(NAME) NAME ICE_SO_VERSION "uwpD.lib"
#         else
#            define ICE_LIBNAME(NAME) NAME ICE_SO_VERSION "D.lib"
#         endif
#      else
#         if defined(ICE_OS_WINRT)
#            define ICE_LIBNAME(NAME) NAME ICE_SO_VERSION "uwp.lib"
#         else
#            define ICE_LIBNAME(NAME) NAME ICE_SO_VERSION ".lib"
#         endif
#      endif
#   endif

//
//  Automatically link with Ice[D|++11|++11D].lib
//
#   if !defined(ICE_STATIC_LIBS) && !defined(ICE_BUILDING_ICE)
#      pragma comment(lib, ICE_LIBNAME("Ice"))
#   endif
#endif

namespace IceUtil
{

//
// By deriving from this class, other classes are made non-copyable.
//
class ICE_API noncopyable
{
protected:

    noncopyable() { }
    ~noncopyable() { } // May not be virtual! Classes without virtual
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

//
// Macros to facilitate C++98 -> C++11 transition
//
#ifdef ICE_CPP11_MAPPING // C++11 mapping
#   include <memory>
#   include <future>
#   define ICE_HANDLE ::std::shared_ptr
#   define ICE_INTERNAL_HANDLE ::std::shared_ptr
#   define ICE_PROXY_HANDLE ::std::shared_ptr
#   define ICE_MAKE_SHARED(T, ...) ::std::make_shared<T>(__VA_ARGS__)
#   define ICE_DEFINE_PTR(TPtr, T) typedef ::std::shared_ptr<T> TPtr
#   define ICE_ENUM(CLASS,ENUMERATOR) CLASS::ENUMERATOR
#   define ICE_NULLPTR nullptr
#   define ICE_DYNAMIC_CAST(T,V) ::std::dynamic_pointer_cast<T>(V)
#   define ICE_SHARED_FROM_THIS shared_from_this()
#   define ICE_CHECKED_CAST(T, ...) Ice::checkedCast<T>(__VA_ARGS__)
#   define ICE_UNCHECKED_CAST(T, ...) Ice::uncheckedCast<T>(__VA_ARGS__)
#   define ICE_VALUE_FACTORY ::std::function<::std::shared_ptr<::Ice::Value> (const std::string&)>
#   define ICE_CLOSE_CALLBACK ::std::function<void (const ::std::shared_ptr<::Ice::Connection>&)>
#   define ICE_HEARTBEAT_CALLBACK ::std::function<void (const ::std::shared_ptr<::Ice::Connection>&)>
#   define ICE_IN(T) T
#   define ICE_EXCEPTION_ISSET(T) T
#   define ICE_RETHROW_EXCEPTION(T) ::std::rethrow_exception(T)
#   define ICE_RESET_EXCEPTION(T, V)  T = V
#else // C++98 mapping
#   define ICE_HANDLE ::IceUtil::Handle
#   define ICE_INTERNAL_HANDLE ::IceInternal::Handle
#   define ICE_PROXY_HANDLE ::IceInternal::ProxyHandle
#   define ICE_MAKE_SHARED(T, ...) new T(__VA_ARGS__)
#   define ICE_DEFINE_PTR(TPtr, T) typedef ::IceUtil::Handle<T> TPtr
#   define ICE_ENUM(CLASS,ENUMERATOR) ENUMERATOR
#   define ICE_NULLPTR 0
#   define ICE_DYNAMIC_CAST(T,V) T##Ptr::dynamicCast(V)
#   define ICE_SHARED_FROM_THIS this
#   define ICE_CHECKED_CAST(T, ...) T::checkedCast(__VA_ARGS__)
#   define ICE_UNCHECKED_CAST(T, ...) T::uncheckedCast(__VA_ARGS__)
#   define ICE_VALUE_FACTORY ::Ice::ValueFactoryPtr
#   define ICE_CLOSE_CALLBACK ::Ice::CloseCallbackPtr
#   define ICE_HEARTBEAT_CALLBACK ::Ice::HeartbeatCallbackPtr
#   define ICE_IN(T) const T&
#   define ICE_EXCEPTION_ISSET(T) (T.get() != 0)
#   define ICE_RETHROW_EXCEPTION(T) T->ice_throw()
#   define ICE_RESET_EXCEPTION(T,V) T.reset(V)
#endif

#endif
