// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_UTIL_DISABLEWARNINGS_H
#define ICE_UTIL_DISABLEWARNINGS_H

//
// This header file disables or makes non-fatal various compiler warnings that
// we don't want.
//
// IMPORTANT: Do *not* include this header file in another public header file!
//            Doing this may potentially disable the warnings in the source
//            code of our customers, which would be bad. Only include this
//            header file in Ice *source* files!
//

//
// Microsoft Visual C++
//
#if defined(_MSC_VER)
#    define _CRT_SECURE_NO_DEPRECATE 1  // C4996 '<C function>' was declared deprecated
#    pragma warning( 4 : 4996 ) // C4996 '<function>' was declared deprecated
#    pragma warning( 4 : 4800 ) // C4800 forcing value to bool 'true' or 'false' (performance warning)

#    if (_MSC_VER < 1700)
#       pragma warning( 4 : 4355 ) // C4355 'this' : used in base member initializer list
#    endif
#endif


//
// GCC
//
#if defined(__GNUC__)
#   pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

//
// Clang
//
#if defined(__clang__)
#   pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif


#endif
