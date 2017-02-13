// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <Ice/Locator.h>
#include <Ice/Router.h>
#include <TestCommon.h>
#include <Test.h>

using namespace std;

Test::PriorityPrx
allTests(const Ice::CommunicatorPtr& communicator)
{
    cout << "testing server priority... " << flush;
    string ref = "test:default -p 12010 -t 10000";
    Ice::ObjectPrx base = communicator->stringToProxy(ref);
    test(base);

    Test::PriorityPrx priority = Test::PriorityPrx::checkedCast(base);

    try
    {
#ifdef _WIN32
        test(1 == priority->getPriority());
#else
        test(50 == priority->getPriority());
#endif
    }
    catch(...)
    {
        test(false);
    }
    cout << "ok" << endl;

    return priority;
}

