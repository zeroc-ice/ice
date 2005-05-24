// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TEST_COMMON_H
#define TEST_COMMON_H

#include <IceUtil/Config.h>

#include <stdio.h>

void
inline testFailed(const char* expr, const char* file, unsigned int line)
{
    printf("failed!\n");
    printf("%s:%d: assertion `%s' failed\n", file, line, expr);
    abort();
}

#define test(ex) ((ex) ? ((void)0) : testFailed(#ex, __FILE__, __LINE__))

#endif
