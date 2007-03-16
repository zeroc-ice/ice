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
using namespace Test;

RetryPrx
allTests(const Ice::CommunicatorPtr& communicator)
{
    tprintf("testing stringToProxy... ");
    string ref = communicator->getProperties()->getPropertyWithDefault(
        "Retry.Proxy", "retry:default -p 12010 -t 10000");
    Ice::ObjectPrx base1 = communicator->stringToProxy(ref);
    test(base1);
    Ice::ObjectPrx base2 = communicator->stringToProxy(ref);
    test(base2);
    tprintf("ok\n");

    tprintf("testing checked cast... ");
    RetryPrx retry1 = RetryPrx::checkedCast(base1);
    test(retry1);
    test(retry1 == base1);
    RetryPrx retry2 = RetryPrx::checkedCast(base2);
    test(retry2);
    test(retry2 == base2);
    tprintf("ok\n");

    tprintf("calling regular operation with first proxy... ");
    retry1->op(false);
    tprintf("ok\n");

    tprintf("calling operation to kill connection with second proxy... ");
    try
    {
	retry2->op(true);
	test(false);
    }
    catch(Ice::ConnectionLostException)
    {
	tprintf("ok\n");
    }

    tprintf("calling regular operation with first proxy again... ");
    retry1->op(false);
    tprintf("ok\n");

    return retry1;
}
