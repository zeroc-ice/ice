// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <TestSuite.h>
#include <MutexTest.h>
#include <RecMutexTest.h>
#include <StaticMutexTest.h>
#include <CreateTest.h>
#include <AliveTest.h>
#include <StartTest.h>
#include <MonitorMutexTest.h>
#include <MonitorRecMutexTest.h>

std::list<TestBasePtr> allTests;

void
initializeTestSuite()
{
    allTests.push_back(new MutexTest);
    allTests.push_back(new StartTest);
    allTests.push_back(new CreateTest);
    allTests.push_back(new AliveTest);
    allTests.push_back(new RecMutexTest);
    allTests.push_back(new StaticMutexTest);
    allTests.push_back(new MonitorMutexTest);
    allTests.push_back(new MonitorRecMutexTest);
}
