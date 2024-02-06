//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <TestBase.h>
#include <list>
#include <CountDownLatchTest.h>
#include <MutexTest.h>
#include <CreateTest.h>
#include <StartTest.h>
#include <SleepTest.h>
#include <MonitorMutexTest.h>

std::list<TestBasePtr> allTests;

void
initializeTestSuite()
{
    allTests.push_back(new MutexTest);
    allTests.push_back(new CountDownLatchTest);
    allTests.push_back(new StartTest);
    allTests.push_back(new SleepTest);
    allTests.push_back(new CreateTest);
    allTests.push_back(new MonitorMutexTest);
}
