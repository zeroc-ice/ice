// Copyright (c) ZeroC, Inc.

#ifndef ICE_DISABLE_WARNINGS_H
#define ICE_DISABLE_WARNINGS_H

// Disable mostly deprecated warnings.

//
// Microsoft Visual C++
//
#if defined(_MSC_VER)
#    define _CRT_SECURE_NO_DEPRECATE 1 // C4996 '<C function>' was declared deprecated
#    pragma warning(disable : 4996)    // C4996 '<function>' was declared deprecated
#    pragma warning(disable : 4800)    // C4800 forcing value to bool 'true' or 'false' (performance warning)
#endif

//
// GCC
//
#if defined(__GNUC__)
#    pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

//
// Clang
//
#if defined(__clang__)
#    pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif

#endif
