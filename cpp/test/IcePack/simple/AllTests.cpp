// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestCommon.h>
#include <Test.h>

using namespace std;

TestPrx
allTests(const Ice::CommunicatorPtr& communicator)
{
    cout << "testing stringToProxy... " << flush;
    string ref = "test @ TestAdapter";
    Ice::ObjectPrx base = communicator->stringToProxy(ref);
    test(base);
    cout << "ok" << endl;

    cout << "testing checked cast... " << flush;
    TestPrx obj = TestPrx::checkedCast(base);
    test(obj);
    test(obj == base);
    cout << "ok" << endl;

    cout << "pinging server... " << flush;
    obj->ice_ping();
    cout << "ok" << endl;

    return obj;
}
