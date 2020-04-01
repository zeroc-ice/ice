//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_UTIL_SCANNER_CONFIG_H
#define ICE_UTIL_SCANNER_CONFIG_H

#include <IceUtil/Config.h> // Required by generated Scanners.

#if defined(_MSC_VER)
// Suppresses inclusion of the non ANSI unistd.h header file.
#   define YY_NO_UNISTD_H

// in C99 conformant compilers we don't need to include it because the
// header is included by inttypes.h, that is included by the generated
// Scanners.
#   include <stdint.h>

// '<' : signed/unsigned mismatch
#   pragma warning(disable:4018)
// 'initializing' : conversion from '__int64' to 'int', possible loss of data
#   pragma warning(disable:4244)
// unreferenced local function has been removed
#   pragma warning(disable:4505)

#   if defined(ICE_64)
// '=' : conversion from 'size_t' to 'int', possible loss of data
// The result of fread() is a size_t and gets inserted into an int
#       pragma warning(disable:4267)
#   endif
#endif

#if defined(__GNUC__)
#   pragma GCC diagnostic ignored "-Wsign-compare"
#   pragma GCC diagnostic ignored "-Wunused-function"
#   pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#endif

#if defined(__clang__)
#   pragma clang diagnostic ignored "-Wconversion"
#   pragma clang diagnostic ignored "-Wsign-conversion"
#   pragma clang diagnostic ignored "-Wdocumentation"
#   pragma clang diagnostic ignored "-Wunused-function"
#endif

#endif
