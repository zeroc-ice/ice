// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestCommon.h>
#include <Test.h>

using namespace std;

void
allTests(const Ice::CommunicatorPtr& communicator, int port, int lastPort)
{
    cout << "testing stringToProxy... " << flush;
    ostringstream ref;
    ref << "test:default -t 2000 -p " << port;
    Ice::ObjectPrx base = communicator->stringToProxy(ref.str());
    test(base);
    ostringstream lastRef;
    lastRef << "test:default -t 2000 -p " << lastPort;
    Ice::ObjectPrx lastBase = communicator->stringToProxy(lastRef.str());
    test(lastBase);
    cout << "ok" << endl;

    cout << "testing checked cast... " << flush;
    TestPrx obj = TestPrx::checkedCast(base);
    test(obj);
    test(obj == base);
    TestPrx lastObj = TestPrx::checkedCast(lastBase);
    test(lastObj);
    test(lastObj == lastBase);
    cout << "ok" << endl;

    cout << "shutting down all servers with single call... " << flush;
    obj->shutdown();
    cout << "ok" << endl;

    cout << "testing whether all servers are gone... " << flush;
    try
    {
	lastObj->ice_ping();
	test(false);
    }
    catch(const Ice::SocketException&)
    {
	cout << "ok" << endl;
    }    
}
