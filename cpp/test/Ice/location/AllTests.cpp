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
allTests(const Ice::CommunicatorPtr& communicator, const string& ref)
{
    ServerManagerPrx manager = ServerManagerPrx::checkedCast(communicator->stringToProxy(ref));
    test(manager);

    //
    // Start a server, get the port of the adapter it's listening on,
    // and add it to the configuration so that the client can locate
    // the TestAdapter adapter.
    //
    cout << "starting server... " << flush;
    manager->startServer();
    cout << "ok" << endl;

    cout << "testing stringToProxy... " << flush;
    Ice::ObjectPrx base = communicator->stringToProxy("test @ TestAdapter");
    Ice::ObjectPrx base2 = communicator->stringToProxy("test @ TestAdapter");
    cout << "ok" << endl;

    cout << "testing checked cast... " << flush;
    TestPrx obj = TestPrx::checkedCast(base);
    obj = TestPrx::checkedCast(communicator->stringToProxy("test@TestAdapter"));
    obj = TestPrx::checkedCast(communicator->stringToProxy("test   @TestAdapter"));
    obj = TestPrx::checkedCast(communicator->stringToProxy("test@   TestAdapter"));
    test(obj);
    TestPrx obj2 = TestPrx::checkedCast(base2);
    test(obj2);
    cout << "ok" << endl;
 
    cout << "testing object reference from server... " << flush;
    HelloPrx hello = obj->getHello();
    hello->sayHello();
    cout << "ok" << endl;

    cout << "shutdown server... " << flush;
    obj->shutdown();
    cout << "ok" << endl;

    cout << "restarting server... " << flush;
    manager->startServer();
    cout << "ok" << endl;

    cout << "testing whether server is still reachable... " << flush;
    try
    {
	obj2->ice_ping();
    }
    catch(const Ice::SocketException&)
    {
	test(false);
    }
    cout << "ok" << endl;
    
    
    cout << "testing object reference from server... " << flush;
    hello->sayHello();
    cout << "ok" << endl;

    cout << "testing reference with unknown adapter... " << flush;
    try
    {
	base = communicator->stringToProxy("test @ TestAdapterUnknown");
	base->ice_ping();
	test(false);
    }
    catch (const Ice::NoEndpointException&)
    {
    }
    cout << "ok" << endl;

    cout << "shutdown server... " << flush;
    obj->shutdown();
    cout << "ok" << endl;

    cout << "testing whether server is gone... " << flush;
    try
    {
	obj2->ice_ping();
	test(false);
    }
    catch(const Ice::SocketException&)
    {
	cout << "ok" << endl;
    }

    cout << "shutdown server manager..." << flush;
    manager->shutdown();
    cout << "ok" << endl;
}
