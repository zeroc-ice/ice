// **********************************************************************
//
// Copyright (c) 2002
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
using namespace Ice;

TestPrx
allTests(const CommunicatorPtr& communicator)
{
    cout << "testing stringToProxy... " << flush;
    ObjectPrx base = communicator->stringToProxy("test:default -p 12345 -t 2000");
    test(base);
    cout << "ok" << endl;

    cout << "testing checked cast... " << flush;
    TestPrx obj = TestPrx::checkedCast(base);
    test(obj);
    test(obj == base);
    cout << "ok" << endl;

    cout << "deactivating object adapter in the server... " << flush;
    obj->deactivate();
    cout << "ok" << endl;

    return obj;
}
