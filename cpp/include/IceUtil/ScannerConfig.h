//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_UTIL_SCANNER_CONFIG_H
#define ICE_UTIL_SCANNER_CONFIG_H

#include <IceUtil/Config.h> // Required by generated Scanners.

//
// in C99 conformant compilers we don't need to include it because the
// header is included by inttypes.h, that is included by the gernated
// Scanners.
//
#if defined(_MSC_VER)
#   include <stdint.h>
#endif

//
// Clang++ >= 5.1 and VC++ using C++17 standard deprecate 'register' storage
// class specifier used by lex generated Scanners.
//
#if defined(__clang__)
#   pragma clang diagnostic ignored "-Wdeprecated-register"
#elif defined(_MSC_VER)
#   pragma warning(disable:5033)
#endif

#if defined(__clang__)
#   pragma clang diagnostic ignored "-Wconversion"
#   pragma clang diagnostic ignored "-Wsign-conversion"
#   pragma clang diagnostic ignored "-Wdocumentation"
#endif

#endif
