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
#include <IcePack/Admin.h>
#include <TestCommon.h>
#include <Test.h>

using namespace std;

TestPrx
allTests(const Ice::CommunicatorPtr& communicator)
{
    cout << "testing stringToProxy... " << flush;
    Ice::ObjectPrx base = communicator->stringToProxy("test @ TestAdapter");
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

TestPrx
allTestsWithDeploy(const Ice::CommunicatorPtr& communicator)
{
    cout << "testing stringToProxy... " << flush;
    Ice::ObjectPrx base = communicator->stringToProxy("test @ TestAdapter");
    test(base);
    Ice::ObjectPrx base2 = communicator->stringToProxy("test");
    test(base2);
    cout << "ok" << endl;

    cout << "testing checked cast... " << flush;
    TestPrx obj = TestPrx::checkedCast(base);
    test(obj);
    test(obj == base);
    TestPrx obj2 = TestPrx::checkedCast(base2);
    test(obj2);
    test(obj2 == base2);
    cout << "ok" << endl;

    cout << "pinging server... " << flush;
    obj->ice_ping();
    obj2->ice_ping();
    cout << "ok" << endl;

    cout << "testing reference with unknown identity... " << flush;
    try
    {
	communicator->stringToProxy("unknown/unknown")->ice_ping();
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
	communicator->stringToProxy("test @ TestAdapterUnknown")->ice_ping();
	test(false);
    }
    catch (const Ice::NotRegisteredException& ex)
    {
	test(ex.kindOfObject == "object adapter");
	test(ex.id == "TestAdapterUnknown");
    }
    cout << "ok" << endl;

    IcePack::AdminPrx admin = IcePack::AdminPrx::checkedCast(communicator->stringToProxy("IcePack/Admin"));
    test(admin);

    admin->setServerActivation("server", IcePack::Manual);
    admin->stopServer("server");

    cout << "testing whether server is still reachable... " << flush;
    try
    {
	obj = TestPrx::checkedCast(base);
	test(false);
    }
    catch(const Ice::NoEndpointException&)
    {
    }
    try
    {
	obj2 = TestPrx::checkedCast(base2);
	test(false);
    }
    catch(const Ice::NoEndpointException&)
    {
    }
    
    admin->setServerActivation("server", IcePack::OnDemand);

    try
    {
	obj = TestPrx::checkedCast(base);
    }
    catch(const Ice::NoEndpointException&)
    {
	test(false);
    }
    try
    {
	obj2 = TestPrx::checkedCast(base2);
    }
    catch(const Ice::NoEndpointException&)
    {
	test(false);
    }
    cout << "ok" << endl;    

    return obj;
}
