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

#ifndef TEST_SUITE_H
#define TEST_SUITE_H

#include <TestBase.h>
#include <list>

extern std::list<TestBasePtr> allTests;

void initializeTestSuite();

#endif

