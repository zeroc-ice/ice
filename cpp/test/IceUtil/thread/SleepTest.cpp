//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <IceUtil/IceUtil.h>

#include <stdio.h>

#include <SleepTest.h>
#include <TestHelper.h>

using namespace std;
using namespace IceUtil;

static const string createTestName("thread sleep");

SleepTest::SleepTest() :
    TestBase(createTestName)
{
}

void
SleepTest::run()
{
    try
    {
        IceUtil::ThreadControl::sleep(IceUtil::Time::secondsDouble(-1));
        test(false);
    }
    catch(const InvalidTimeoutException&)
    {
    }

#ifdef _WIN32
    try
    {
        IceUtil::ThreadControl::sleep(IceUtil::Time::secondsDouble(INFINITE));
        test(false);
    }
    catch(const InvalidTimeoutException&)
    {
    }
#endif
}
