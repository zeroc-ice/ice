// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <TestSuite.h>
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
