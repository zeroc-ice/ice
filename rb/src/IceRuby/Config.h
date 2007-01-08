// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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
#endif

extern "C"
{
typedef VALUE(*ICE_RUBY_ENTRY_POINT)(...);
}

#define CAST_METHOD(X) reinterpret_cast<ICE_RUBY_ENTRY_POINT>(X)

#endif
