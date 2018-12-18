// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <Ice/Locator.h>
#include <Ice/Router.h>
#include <TestHelper.h>
#include <Test.h>

using namespace std;

Test::PriorityPrxPtr
allTests(Test::TestHelper* helper)
{
    Ice::CommunicatorPtr communicator = helper->communicator();
    cout << "testing server priority... " << flush;
    string ref = "test:" + helper->getTestEndpoint() + " -t 10000";
    Ice::ObjectPrxPtr base = communicator->stringToProxy(ref);
    test(base);

    Test::PriorityPrxPtr priority = ICE_UNCHECKED_CAST(Test::PriorityPrx, base);

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
