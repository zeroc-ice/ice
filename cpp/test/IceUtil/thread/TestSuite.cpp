//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <TestBase.h>
#include <list>
#include <CountDownLatchTest.h>
#include <CreateTest.h>
#include <StartTest.h>

using namespace std;

std::list<TestBasePtr> allTests;

void
initializeTestSuite()
{
    allTests.push_back(make_shared<CountDownLatchTest>());
    allTests.push_back(make_shared<StartTest>());
    allTests.push_back(make_shared<CreateTest>());
}
