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

void
allTests(const Ice::CommunicatorPtr& communicator, const string& ref)
{
    ServerManagerPrx manager = ServerManagerPrx::checkedCast(communicator->stringToProxy(ref));
    test(manager);

    cout << "testing stringToProxy... " << flush;
    Ice::ObjectPrx base = communicator->stringToProxy("test @ TestAdapter");
    Ice::ObjectPrx base2 = communicator->stringToProxy("test @ TestAdapter");
    Ice::ObjectPrx base3 = communicator->stringToProxy("test");
    Ice::ObjectPrx base4 = communicator->stringToProxy("ServerManager");
    cout << "ok" << endl;

    cout << "starting server... " << flush;
    manager->startServer();
    cout << "ok" << endl;

    cout << "testing checked cast... " << flush;
    TestPrx obj = TestPrx::checkedCast(base);
    obj = TestPrx::checkedCast(communicator->stringToProxy("test@TestAdapter"));
    obj = TestPrx::checkedCast(communicator->stringToProxy("test   @TestAdapter"));
    obj = TestPrx::checkedCast(communicator->stringToProxy("test@   TestAdapter"));
    test(obj);
    TestPrx obj2 = TestPrx::checkedCast(base2);
    test(obj2);
    TestPrx obj3 = TestPrx::checkedCast(base3);
    test(obj3);
    ServerManagerPrx obj4 = ServerManagerPrx::checkedCast(base4);
    test(obj4);
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
	obj2 = TestPrx::checkedCast(base2);
	obj2->ice_ping();
    }
    catch(const Ice::LocalException&)
    {
	test(false);
    }
    cout << "ok" << endl;    
    
    cout << "testing object reference from server... " << flush;
    hello->sayHello();
    cout << "ok" << endl;

    cout << "testing reference with unknown identity... " << flush;
    try
    {
	base = communicator->stringToProxy("unknown/unknown");
	base->ice_ping();
	test(false);
    }
    catch (const Ice::NotRegisteredException& ex)
    {
	test(ex.kindOfObject == "object");
	test(ex.id == "unknown/unknown");
    }
    cout << "ok" << endl;

    cout << "testing reference with unknown adapter... " << flush;
    try
    {
	base = communicator->stringToProxy("test @ TestAdapterUnknown");
	base->ice_ping();
	test(false);
    }
    catch (const Ice::NotRegisteredException& ex)
    {
	test(ex.kindOfObject == "object adapter");
	test(ex.id == "TestAdapterUnknown");
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
    catch(const Ice::LocalException&)
    {
	cout << "ok" << endl;
    }

    cout << "shutdown server manager..." << flush;
    manager->shutdown();
    cout << "ok" << endl;
}
