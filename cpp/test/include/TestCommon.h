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

#ifndef TEST_COMMON_H
#define TEST_COMMON_H

#include <IceUtil/Config.h>
#include <cstdlib>

void
inline testFailed(const char* expr, const char* file, unsigned int line)
{
    std::cout << "failed!" << std::endl;
    std::cout << file << ':' << line << ": assertion `" << expr << "' failed" << std::endl;
    abort();
}

#define test(ex) ((ex) ? ((void)0) : testFailed(#ex, __FILE__, __LINE__))

#endif
