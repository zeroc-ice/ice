// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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

void
allTests(const Ice::CommunicatorPtr& communicator)
{
    cout << "testing stringToProxy... " << flush;
    string ref = "hold:default -p 12010 -t 10000";
    Ice::ObjectPrx base = communicator->stringToProxy(ref);
    test(base);
    cout << "ok" << endl;

    cout << "testing checked cast... " << flush;
    HoldPrx hold = HoldPrx::checkedCast(base);
    test(hold);
    test(hold == base);
    cout << "ok" << endl;

    cout << "changing state between active and hold rapidly... " << flush;
    for(int i = 0; i < 100; ++i)
    {
        hold->putOnHold(0);
    }
    cout << "ok" << endl;

    cout << "changing state to hold and shutting down server... " << flush;
    hold->shutdown();
    cout << "ok" << endl;
}
