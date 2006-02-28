// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestCommon.h>
#include <Test.h>

using namespace std;
using namespace Test;

RetryPrx
allTests(const Ice::CommunicatorPtr& communicator)
{
    cout << "testing stringToProxy... " << flush;
    string ref = "retry:default -p 12010 -t 10000";
    Ice::ObjectPrx base1 = communicator->stringToProxy(ref);
    test(base1);
    Ice::ObjectPrx base2 = communicator->stringToProxy(ref);
    test(base2);
    cout << "ok" << endl;

    cout << "testing checked cast... " << flush;
    RetryPrx retry1 = RetryPrx::checkedCast(base1);
    test(retry1);
    test(retry1 == base1);
    RetryPrx retry2 = RetryPrx::checkedCast(base2);
    test(retry2);
    test(retry2 == base2);
    cout << "ok" << endl;

    cout << "calling regular operation with first proxy... ";
    retry1->op(false);
    cout << "ok" << endl;

    cout << "calling operation to kill connection with second proxy... ";
    try
    {
	retry2->op(true);
	test(false);
    }
    catch(Ice::ConnectionLostException)
    {
	cout << "ok" << endl;
    }

    cout << "calling regular operation with first proxy again... ";
    retry1->op(false);
    cout << "ok" << endl;

    return retry1;
}
