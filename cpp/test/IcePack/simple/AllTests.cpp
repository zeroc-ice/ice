// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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
    string ref;
    
    Ice::PropertiesPtr properties = communicator->getProperties();

    string protocol = properties->getProperty("Ice.Protocol");
    string secure;

    if (protocol.empty())
    {
        protocol = "tcp";
    }

    if (protocol.compare("ssl") == 0)
    {
        secure = " -s ";
    }

    string endpts = protocol + " -p 12346 -t 2000";

    ref = "test" + secure + ":" + endpts;

    Ice::ObjectPrx base = communicator->stringToProxy(ref);
    test(base);
    cout << "ok" << endl;

    cout << "testing checked cast... " << flush;
    TestPrx obj = TestPrx::checkedCast(base);
    test(obj);
    test(obj == base);
    cout << "ok" << endl;

    cout << "pinging server... " << flush;
    obj->_ice_ping();
    cout << "ok" << endl;

    return obj;
}
