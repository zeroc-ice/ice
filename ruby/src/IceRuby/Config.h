// Copyright (c) ZeroC, Inc.

#ifndef ICE_RUBY_CONFIG_H
#define ICE_RUBY_CONFIG_H

//
// This file includes <ruby.h> and should always be included first.
//

#include "Ice/Config.h"

// The ruby.h check for the isfinite macro fails with some C++ standard libraries
// (libc++ > 4000) because the isfinite macro included from the C library's
// math.h is undefined and replaced with a function. As a result, Ruby defines isfinite
// as the finite macro which is deprecated on some platforms (macOS >= 10.9).
// The warning ends up causing a build failure. We define the HAVE_ISFINITE macro here to
// ensure ruby.h doesn't redefine it.
#if defined(__clang__) && defined(_LIBCPP_VERSION) && (_LIBCPP_VERSION >= 4000)
#    define HAVE_ISFINITE 1
#endif

//
// BUGFIX: Workaround unused parameter in ruby.h header file
//
#if defined(__clang__)
// BUGFIX: Workaround clang 13 warnings during ruby macro expansion, it is important to put this before the push/pop
// directives to keep the warnings ignored in the source files where macros are expanded.
#    if __clang_major__ >= 13
#        pragma clang diagnostic ignored "-Wcompound-token-split-by-macro"
#    endif
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wunused-parameter"
//
// BUGFIX: Workaround clang conversion warnings in ruby headers
//
#    pragma clang diagnostic ignored "-Wconversion"

// Silence warnings regarding missing deprecation attributes in ruby headers
#    pragma clang diagnostic ignored "-Wdocumentation"
#    pragma clang diagnostic ignored "-Wdocumentation-deprecated-sync"
#elif defined(__GNUC__)
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wunused-parameter"
// BUGFIX Workaround G++ 10 problem with ruby headers
// see https://bugs.ruby-lang.org/issues/16930
#    pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

#include <ruby.h>
#include <ruby/encoding.h>

#if defined(__clang__)
#    pragma clang diagnostic pop
#elif defined(__GNUC__)
#    pragma GCC diagnostic pop
#endif

//
// The Ruby header file win32/win32.h defines a number of macros for
// functions like shutdown() and close() that wreak havoc.
//
#ifdef _WIN32
#    undef shutdown
#    undef close
#    undef read
#    undef write
#    undef sleep
#endif

extern "C"
{
    typedef VALUE (*ICE_RUBY_ENTRY_POINT)(...);
}

#define CAST_METHOD(X) reinterpret_cast<ICE_RUBY_ENTRY_POINT>(X)

#endif
