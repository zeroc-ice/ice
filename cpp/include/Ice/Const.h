// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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
