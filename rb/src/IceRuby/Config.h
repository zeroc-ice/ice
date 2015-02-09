// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
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
#include <ruby/version.h>

#if defined(RUBY_API_VERSION_CODE) && RUBY_API_VERSION_CODE >= 20200
//
// Since ruby 2.2.0 the macros to convert Bignum to long are 
// not public. We define here our own version.
//
# ifndef BDIGIT
#  if SIZEOF_INT*2 <= SIZEOF_LONG_LONG
#   define BDIGIT unsigned int
#   define SIZEOF_BDIGIT SIZEOF_INT
#   define BDIGIT_DBL unsigned LONG_LONG
#  elif SIZEOF_INT*2 <= SIZEOF_LONG
#   define BDIGIT unsigned int
#   define SIZEOF_BDIGIT SIZEOF_INT
#   define BDIGIT_DBL unsigned long
#  elif SIZEOF_SHORT*2 <= SIZEOF_LONG
#   define BDIGIT unsigned short
#   define SIZEOF_BDIGIT SIZEOF_SHORT
#   define BDIGIT_DBL unsigned long
#  else
#   define BDIGIT unsigned short
#   define SIZEOF_BDIGIT (SIZEOF_LONG/2)
#   define SIZEOF_ACTUAL_BDIGIT SIZEOF_LONG
#   define BDIGIT_DBL unsigned long
#  endif
# endif
# ifndef SIZEOF_ACTUAL_BDIGIT
#  define SIZEOF_ACTUAL_BDIGIT SIZEOF_BDIGIT
# endif

# define BIGNUM_EMBED_LEN_NUMBITS 3
# ifndef BIGNUM_EMBED_LEN_MAX
#  if (SIZEOF_VALUE*3/SIZEOF_ACTUAL_BDIGIT) < (1 << BIGNUM_EMBED_LEN_NUMBITS)-1
#    define BIGNUM_EMBED_LEN_MAX (SIZEOF_VALUE*3/SIZEOF_ACTUAL_BDIGIT)
#  else
#    define BIGNUM_EMBED_LEN_MAX ((1 << BIGNUM_EMBED_LEN_NUMBITS)-1)
#  endif
# endif

struct RBignum {
    struct RBasic basic;
    union {
        struct {
            size_t len;
            BDIGIT *digits;
        } heap;
        BDIGIT ary[BIGNUM_EMBED_LEN_MAX];
    } as;
};

# define BIGNUM_SIGN_BIT FL_USER1
/* sign: positive:1, negative:0 */
# define BIGNUM_SIGN(b) ((RBASIC(b)->flags & BIGNUM_SIGN_BIT) != 0)
# define BIGNUM_SET_SIGN(b,sign) \
  ((sign) ? (RBASIC(b)->flags |= BIGNUM_SIGN_BIT) \
          : (RBASIC(b)->flags &= ~BIGNUM_SIGN_BIT))
# define BIGNUM_POSITIVE_P(b) BIGNUM_SIGN(b)
# define BIGNUM_NEGATIVE_P(b) (!BIGNUM_SIGN(b))

# define BIGNUM_EMBED_FLAG FL_USER2
# define BIGNUM_EMBED_LEN_MASK (FL_USER5|FL_USER4|FL_USER3)
# define BIGNUM_EMBED_LEN_SHIFT (FL_USHIFT+BIGNUM_EMBED_LEN_NUMBITS)
# define BIGNUM_LEN(b) \
    ((RBASIC(b)->flags & BIGNUM_EMBED_FLAG) ? \
     (long)((RBASIC(b)->flags >> BIGNUM_EMBED_LEN_SHIFT) & \
            (BIGNUM_EMBED_LEN_MASK >> BIGNUM_EMBED_LEN_SHIFT)) : \
     RBIGNUM(b)->as.heap.len)
/* LSB:BIGNUM_DIGITS(b)[0], MSB:BIGNUM_DIGITS(b)[BIGNUM_LEN(b)-1] */
# define BIGNUM_DIGITS(b) \
    ((RBASIC(b)->flags & BIGNUM_EMBED_FLAG) ? \
     RBIGNUM(b)->as.ary : \
     RBIGNUM(b)->as.heap.digits)
# define BIGNUM_LENINT(b) rb_long2int(BIGNUM_LEN(b))

# define RBIGNUM(obj) (R_CAST(RBignum)(obj))

# define RBIGNUM_LEN(v) BIGNUM_LEN(v)
# define RBIGNUM_DIGITS(v) BIGNUM_DIGITS(v)
# define SIZEOF_BDIGITS SIZEOF_BDIGIT
#endif

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

#ifndef RBIGNUM_LEN
#   define RBIGNUM_LEN(v) RBIGNUM(v)->len
#endif

//
// The definition of RBIGNUM_DIGITS in 1.8.7p248+ causes a compilation error (see bug 4653),
// so we undefine it and use our own definition below. Note that the macro HAVE_RUBY_RUBY_H
// is only defined in Ruby 1.9.
//
#ifndef HAVE_RUBY_RUBY_H
#   undef RBIGNUM_DIGITS
#endif

#ifndef RBIGNUM_DIGITS
#   define RBIGNUM_DIGITS(v) ((BDIGIT*)RBIGNUM(v)->digits)
#endif

#ifndef RBIGNUM_SIGN
#   define RBIGNUM_SIGN(v) RBIGNUM(v)->sign
#endif

#endif
