//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_UTIL_DISABLEWARNINGS_H
#define ICE_UTIL_DISABLEWARNINGS_H

//
// This header file disables various compiler warnings that we don't want.
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
#    pragma warning(disable:4996) // C4996 '<function>' was declared deprecated
#    pragma warning(disable:4800) // C4800 forcing value to bool 'true' or 'false' (performance warning)
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
