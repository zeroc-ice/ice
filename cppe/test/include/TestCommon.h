// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TEST_COMMON_H
#define TEST_COMMON_H

#include <IceE/Config.h>
#include <IceE/IceE.h>

#ifdef ICEE_TEST_COMMON_API_EXPORTS
#   define ICEE_TEST_COMMON_API ICEE_DECLSPEC_EXPORT
#else
#   define ICEE_TEST_COMMON_API ICEE_DECLSPEC_IMPORT
#endif

ICEE_TEST_COMMON_API void tprintf(const char* fmt, ...);

ICEE_TEST_COMMON_API void testFailed(const char*, const char*, unsigned int);

#define test(ex) ((ex) ? ((void)0) : testFailed(#ex, __FILE__, __LINE__))

#endif
