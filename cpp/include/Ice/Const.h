// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#ifndef ICE_CONST_H
#define ICE_CONST_H

//
// TODO: - switch to C++ compiler macros, since this syntax is compiler
//         dependent
//       - define an ICE_WIDTH macro (or ICE_32 and ICE_64?) to avoid doing
//         the 32 vs 64 bit checks in multiple places.

#if defined(_WIN32)
#    define ICE_INT64_LITERAL(n) n##i64
#elif defined(__linux__) && defined(i386) || (defined (__sun) && !defined(__sparcv9))
#    define ICE_INT64_LITERAL(n) n##LL
#elif defined (__sun) && defined(__sparcv9)
#    define ICE_INT64_LITERAL(n) n##L
#else
#    error "Unsupported operating system or platform!"
#endif

#endif
