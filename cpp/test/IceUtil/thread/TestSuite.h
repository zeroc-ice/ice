// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef TEST_SUITE_H
#define TEST_SUITE_H

#include <TestBase.h>
#include <list>

extern std::list<TestBasePtr> allTests;

void initializeTestSuite();

#endif

