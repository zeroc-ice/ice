// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/IceE.h>
#include <IceE/Locator.h>
#include <TestCommon.h>
#include <Test.h>

using namespace std;

Test::MyClassPrx
allTests(const Ice::CommunicatorPtr& communicator, const Ice::InitializationData& initData)
{
    string ref = communicator->getProperties()->getPropertyWithDefault(
	"Operations.Proxy", "test:default -p 12010 -t 10000");
    Ice::ObjectPrx base = communicator->stringToProxy(ref);
    Test::MyClassPrx cl = Test::MyClassPrx::checkedCast(base);
    Test::MyDerivedClassPrx derived = Test::MyDerivedClassPrx::checkedCast(cl);

    tprintf("testing timeout...");
    Test::MyClassPrx clTimeout = Test::MyClassPrx::uncheckedCast(cl->ice_timeout(500));
    try
    {
	clTimeout->opSleep(2000);
	test(false);
    }
    catch(const Ice::TimeoutException&)
    {
    }
    tprintf("ok\n");

    tprintf("testing twoway operations... ");
    void twoways(const Ice::CommunicatorPtr&, const Ice::InitializationData&, const Test::MyClassPrx&);
    twoways(communicator, initData, cl);
    twoways(communicator, initData, derived);
    derived->opDerived();
    tprintf("ok\n");

    tprintf("testing batch oneway operations... ");
    void batchOneways(const Test::MyClassPrx&);
    batchOneways(cl);
    batchOneways(derived);
    tprintf("ok\n");

    return cl;
}
