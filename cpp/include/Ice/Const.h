// **********************************************************************
//
// Copyright (c) 2001
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

#if defined(_WIN32)
#    define ICE_INT64_LITERAL(n) n##i64
#elif defined(__linux__) && defined(i386)
#    define ICE_INT64_LITERAL(n) n##LL
#else
#    error "Unsupported operating system or platform!"
#endif

#endif
