// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#include <TestBase.h>
#include <list>
#include <CountDownLatchTest.h>
#include <MutexTest.h>
#include <RecMutexTest.h>
#include <CreateTest.h>
#include <AliveTest.h>
#include <StartTest.h>
#include <SleepTest.h>
#include <MonitorMutexTest.h>
#include <MonitorRecMutexTest.h>

std::list<TestBasePtr> allTests;

void
initializeTestSuite()
{
    allTests.push_back(new MutexTest);
    allTests.push_back(new CountDownLatchTest);
    allTests.push_back(new StartTest);
    allTests.push_back(new SleepTest);
    allTests.push_back(new CreateTest);
    allTests.push_back(new AliveTest);
    allTests.push_back(new RecMutexTest);
    allTests.push_back(new MonitorMutexTest);
    allTests.push_back(new MonitorRecMutexTest);
}
