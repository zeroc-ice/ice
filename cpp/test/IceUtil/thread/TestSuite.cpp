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

#include <TestSuite.h>
#include <MutexTest.h>
#include <RecMutexTest.h>
#include <RWRecMutexTest.h>
#include <CreateTest.h>
#include <MonitorMutexTest.h>
#include <MonitorRecMutexTest.h>

std::list<TestBasePtr> allTests;

void
initializeTestSuite()
{
    allTests.push_back(new CreateTest);
    allTests.push_back(new MutexTest);
    allTests.push_back(new RecMutexTest);
    allTests.push_back(new RWRecMutexTest);
    allTests.push_back(new MonitorMutexTest);
    allTests.push_back(new MonitorRecMutexTest);
}
