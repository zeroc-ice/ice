// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
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

Test::APrx
allTests(const Ice::CommunicatorPtr& communicator)
{
    cout << "testing classes with circular include... " << flush;

    APrx a;
    try
    {
        string ref = "a:default -p 12010 -t 10000";
        Ice::ObjectPrx base = communicator->stringToProxy(ref);
        a = APrx::uncheckedCast(base);
        a->ice_ping();
    }
    catch(...)
    {
        test(false);
    }

    try
    {
        string ref = "b:default -p 12010 -t 10000";
        Ice::ObjectPrx base = communicator->stringToProxy(ref);
        BPrx b = BPrx::uncheckedCast(base);
        b->ice_ping();
    }
    catch(...)
    {
        test(false);
    }
    cout << "ok" << endl;
    return a;
}
