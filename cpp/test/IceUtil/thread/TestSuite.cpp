// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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
