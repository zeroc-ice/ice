// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef TEST_COMMON_H
#define TEST_COMMON_H

#include <iostream>
#include <cstdlib>

void
inline testFailed(const char* expr, const char* file, unsigned int line)
{
    std::cout << "failed!" << std::endl;
    std::cout << file << ':' << line << ": assertion `" << expr << "' failed"
	      << std::endl;
    abort();
}

#define test(ex) ((ex) ? ((void)0) : testFailed(#ex, __FILE__, __LINE__))

#endif
