// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/IceE.h>
#include <TestCommon.h>
#include <Test.h>

using namespace std;
using namespace Ice;
using namespace Test;

TestIntfPrx
allTests(const CommunicatorPtr& communicator)
{
    printf("testing stringToProxy...");
    string ref = communicator->getProperties()->getPropertyWithDefault("Test.Proxy", "test:default -p 12010 -t 10000");
    ObjectPrx base = communicator->stringToProxy(ref);
    test(base);
    printf("ok\n");

    printf("testing checked cast...");
    TestIntfPrx obj = TestIntfPrx::checkedCast(base);
    test(obj);
    test(obj == base);
    printf("ok\n");

    {
        printf("creating/destroying/recreating object adapter...");
        ObjectAdapterPtr adapter =
            communicator->createObjectAdapterWithEndpoints("TransientTestAdapter", "default -p 9999");
        try
        {
            communicator->createObjectAdapterWithEndpoints("TransientTestAdapter", "default -p 9998");
	    test(false);
        }
        catch(const AlreadyRegisteredException&)
        {
        }
        adapter->destroy();
        adapter = communicator->createObjectAdapterWithEndpoints("TransientTestAdapter", "default -p 9999");
        adapter->destroy();
        printf("ok\n");
    }

    printf("creating/activating/deactivating object adapter in one operation...");
    obj->transient();
    printf("ok\n");

    printf("deactivating object adapter in the server...");
    obj->deactivate();
    printf("ok\n");

    printf("testing whether server is gone...");
    try
    {
	obj->ice_ping();
	test(false);
    }
    catch(const LocalException&)
    {
	printf("ok\n");
    }

    return obj;
}
