// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_RUBY_CONFIG_H
#define ICE_RUBY_CONFIG_H

//
// This file includes <ruby.h> and should always be included first.
//

#include <Ice/Config.h>

//
// COMPILERFIX: This is required to prevent annoying warnings with aCC.
// The aCC -mt option causes the definition of the _POSIX_C_SOURCE macro
// (with another lower value.) and this is causing a warning because of
// the redefinition.
//
//#if defined(__HP_aCC) && defined(_POSIX_C_SOURCE)
//#    undef _POSIX_C_SOURCE
//#endif

//
// Ruby defines _FILE_OFFSET_BITS without a guard; we undefine it to
// avoid a warning
//
#if defined(__SUNPRO_CC) && defined(_FILE_OFFSET_BITS)
   #undef _FILE_OFFSET_BITS
#endif

//
// COMPILERFIX: Mingw headers fail to find some definitions if
// wincrypt.h isn't included before ruby.h
//
#ifdef _WIN32
#   include <wincrypt.h>
#endif

#include <ruby.h>

//
// The Ruby header file win32/win32.h defines a number of macros for
// functions like shutdown() and close() that wreak havoc.
//
#ifdef _WIN32
#   undef shutdown
#   undef close
#   undef read
#   undef write
#   undef sleep
#endif

extern "C"
{
typedef VALUE(*ICE_RUBY_ENTRY_POINT)(...);
}

#define CAST_METHOD(X) reinterpret_cast<ICE_RUBY_ENTRY_POINT>(X)

//
// These macros are defined in Ruby 1.9 but not in 1.8. We define them here
// to maintain compatibility with 1.8.
//
#ifndef RARRAY_PTR
#   define RARRAY_PTR(v) RARRAY(v)->ptr
#endif

#ifndef RARRAY_LEN
#   define RARRAY_LEN(v) RARRAY(v)->len
#endif

#ifndef RSTRING_PTR
#   define RSTRING_PTR(v) RSTRING(v)->ptr
#endif

#ifndef RSTRING_LEN
#   define RSTRING_LEN(v) RSTRING(v)->len
#endif

#ifndef RFLOAT_VALUE
#   define RFLOAT_VALUE(v) RFLOAT(v)->value
#endif

//
// The RARRAY_AREF and RARRAY_ASET macros were added in Ruby 2.1.
//
#ifndef RARRAY_AREF
#   define RARRAY_AREF(a, i) (RARRAY_PTR(a)[i])
#endif

#ifndef RARRAY_ASET
#   define RARRAY_ASET(a, i, v) RARRAY_PTR(a)[i] = v
#endif

#endif
