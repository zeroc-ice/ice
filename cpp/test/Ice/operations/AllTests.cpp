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

Test::MyClassPrx
allTests(const Ice::CommunicatorPtr& communicator)
{
    string ref;
    
    Ice::PropertiesPtr properties = communicator->getProperties();

    string address = properties->getProperty("Ice.Address");
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

    string endpts = protocol + " -p 12345 -t 2000";

    if (!address.empty())
    {
        endpts += " -h " + address;
    }

    ref = "test" + secure + ":" + endpts;

    cout << "testing stringToProxy... " << flush;
    Ice::ObjectPrx base = communicator->stringToProxy(ref);
    test(base);
    cout << "ok" << endl;

    cout << "testing checked cast... " << flush;
    Test::MyClassPrx cl = Test::MyClassPrx::checkedCast(base);
    test(cl);
    Test::MyDerivedClassPrx derived = Test::MyDerivedClassPrx::checkedCast(cl);
    test(derived);
    test(cl == base);
    test(derived == base);
    test(cl == derived);
    cout << "ok" << endl;

    cout << "testing twoway operations... " << flush;
    void twoways(const Test::MyClassPrx&);
    twoways(cl);
    twoways(derived);
    derived->opDerived();
    cout << "ok" << endl;

    return cl;
}
