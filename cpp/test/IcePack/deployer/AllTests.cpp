// **********************************************************************
//
// Copyright (c) 2003
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
#include <Ice/BuiltinSequences.h>
#include <Ice/IdentityUtil.h>
#include <IcePack/Query.h>
#include <IcePack/Admin.h>
#include <TestCommon.h>
#include <Test.h>

using namespace std;

struct ProxyIdentityEqual : public std::binary_function<Ice::ObjectPrx,string,bool>
{

public:

    bool 
    operator()(const Ice::ObjectPrx& p1, const string& id) const
    {
	return p1->ice_getIdentity() == Ice::stringToIdentity(id);
    }
};

void
allCommonTests(const Ice::CommunicatorPtr& communicator)
{
    IcePack::AdminPrx admin = IcePack::AdminPrx::checkedCast(communicator->stringToProxy("IcePack/Admin"));
    test(admin);

    cout << "test server registration..." << flush;
    Ice::StringSeq serverNames = admin->getAllServerNames();
    test(find(serverNames.begin(), serverNames.end(), "Server1") != serverNames.end());
    test(find(serverNames.begin(), serverNames.end(), "Server2") != serverNames.end());
    test(find(serverNames.begin(), serverNames.end(), "IceBox1") != serverNames.end());
    test(find(serverNames.begin(), serverNames.end(), "IceBox2") != serverNames.end());
    cout << "ok" << endl;

    cout << "testing adapter registration... " << flush;
    Ice::StringSeq adapterIds = admin->getAllAdapterIds();
    test(find(adapterIds.begin(), adapterIds.end(), "Server-Server1") != adapterIds.end());
    test(find(adapterIds.begin(), adapterIds.end(), "Server-Server2") != adapterIds.end());
    test(find(adapterIds.begin(), adapterIds.end(), "Service1-IceBox1.Service1") != adapterIds.end());
    test(find(adapterIds.begin(), adapterIds.end(), "IceBox1Service2Adapter") != adapterIds.end());
    test(find(adapterIds.begin(), adapterIds.end(), "Service1-IceBox2.Service1") != adapterIds.end());
    test(find(adapterIds.begin(), adapterIds.end(), "IceBox2Service2Adapter") != adapterIds.end());
    cout << "ok" << endl;

    IcePack::QueryPrx query = IcePack::QueryPrx::checkedCast(communicator->stringToProxy("IcePack/Query"));
    test(query);

    cout << "testing object registration... " << flush;
    Ice::ObjectProxySeq objects = query->findAllObjectsWithType("::Test");
    test(find_if(objects.begin(), objects.end(), bind2nd(ProxyIdentityEqual(),"Server1")) != objects.end());
    test(find_if(objects.begin(), objects.end(), bind2nd(ProxyIdentityEqual(),"Server2")) != objects.end());
    test(find_if(objects.begin(), objects.end(), bind2nd(ProxyIdentityEqual(),"IceBox1-Service1")) != objects.end());
    test(find_if(objects.begin(), objects.end(), bind2nd(ProxyIdentityEqual(),"IceBox1-Service2")) != objects.end());
    test(find_if(objects.begin(), objects.end(), bind2nd(ProxyIdentityEqual(),"IceBox2-Service1")) != objects.end());
    test(find_if(objects.begin(), objects.end(), bind2nd(ProxyIdentityEqual(),"IceBox2-Service2")) != objects.end());

    {
	Ice::ObjectPrx obj = query->findObjectByType("::Test");
	string id = Ice::identityToString(obj->ice_getIdentity());
	test(id == "Server1" || id == "Server2" || 
	     id == "IceBox1-Service1" || id == "IceBox1-Service2" ||
	     id == "IceBox2-Service1" || id == "IceBox2-Service2");
    }

    try
    {
	Ice::ObjectPrx obj = query->findObjectByType("::Foo");
    }
    catch(const IcePack::ObjectNotExistException&)
    {
    }

    cout << "ok" << endl;
}

void 
allTests(const Ice::CommunicatorPtr& communicator)
{
    allCommonTests(communicator);

    //
    // Ensure that all server and service objects are reachable.
    //
    // The identity for the test object in deployed server or services
    // is the name of the service or server. The object adapter name
    // is Adapter prefixed with the name of the service or
    // server. Ensure we can reach each object.
    //
    cout << "pinging server objects... " << flush;

    TestPrx obj;

    obj = TestPrx::checkedCast(communicator->stringToProxy("Server1@Server-Server1"));
    obj = TestPrx::checkedCast(communicator->stringToProxy("Server2@Server-Server2"));
    obj = TestPrx::checkedCast(communicator->stringToProxy("IceBox1-Service1@Service1-IceBox1.Service1"));
    obj = TestPrx::checkedCast(communicator->stringToProxy("IceBox1-Service2@IceBox1Service2Adapter"));
    obj = TestPrx::checkedCast(communicator->stringToProxy("IceBox2-Service1@Service1-IceBox2.Service1"));
    obj = TestPrx::checkedCast(communicator->stringToProxy("IceBox2-Service2@IceBox2Service2Adapter"));
    
    cout << "ok" << endl;

    cout << "testing server configuration... " << flush;

    obj = TestPrx::checkedCast(communicator->stringToProxy("Server1@Server-Server1"));
    test(obj->getProperty("Type") == "Server");
    test(obj->getProperty("Name") == "Server1");

    obj = TestPrx::checkedCast(communicator->stringToProxy("Server2@Server-Server2"));
    test(obj->getProperty("Target1") == "1");
    test(obj->getProperty("Target2") == "1");

    cout << "ok" << endl;

    cout << "testing service configuration... " << flush;

    obj = TestPrx::checkedCast(communicator->stringToProxy("IceBox1-Service1@Service1-IceBox1.Service1"));
    test(obj->getProperty("Service1.Type") == "standard");
    test(obj->getProperty("Service1.ServiceName") == "Service1");
    
    obj = TestPrx::checkedCast(communicator->stringToProxy("IceBox2-Service2@IceBox2Service2Adapter"));
    test(obj->getProperty("Service2.Type") == "freeze");
    test(obj->getProperty("Service2.ServiceName") == "Service2");

    test(obj->getProperty("Service2.DebugProperty") == "");
    test(obj->getProperty("Service1.DebugProperty") == "");
    
    IcePack::AdminPrx admin = IcePack::AdminPrx::checkedCast(communicator->stringToProxy("IcePack/Admin"));
    test(admin);

    //
    // Ping the icebox service manager to avoid terminating the icebox
    // too soon (before the icebox is fully initialized) and some
    // connection warnings message (caused by the fact the termination
    // handler is not yet installed and communicator not properly
    // shutdown).
    //
    IcePack::ServerDescription desc;

    desc = admin->getServerDescription("IceBox1");
    desc.serviceManager->ice_ping();

    desc = admin->getServerDescription("IceBox2");
    desc.serviceManager->ice_ping();

    cout << "ok" << endl;
}

void
allTestsWithTarget(const Ice::CommunicatorPtr& communicator)
{
    allCommonTests(communicator);

    IcePack::AdminPrx admin = IcePack::AdminPrx::checkedCast(
	communicator->stringToProxy("IcePack/Admin"));
    test(admin);

    cout << "pinging server objects... " << flush;

    TestPrx obj;

    admin->setServerActivation("Server1", IcePack::Manual);
    try
    {
	obj = TestPrx::checkedCast(communicator->stringToProxy("Server1@Server-Server1"));
	test(false);
    }
    catch(const Ice::LocalException&)
    {
    }
    admin->startServer("Server1");
    
    obj = TestPrx::checkedCast(communicator->stringToProxy("Server1@Server-Server1"));
    obj = TestPrx::checkedCast(communicator->stringToProxy("Server2@Server-Server2"));

    cout << "ok" << endl;

    cout << "testing service configuration... " << flush;

    obj = TestPrx::checkedCast(communicator->stringToProxy("IceBox1-Service1@Service1-IceBox1.Service1"));
    test(obj->getProperty("Service1.DebugProperty") == "debug");

    //
    // Ping the icebox service manager to avoid terminating the icebox
    // too soon (before the icebox is fully initialized) and some
    // connection warnings message (caused by the fact the termination
    // handler is not yet installed and communicator not properly
    // shutdown).
    //
    IcePack::ServerDescription desc = admin->getServerDescription("IceBox1");
    desc.serviceManager->ice_ping();

    cout << "ok" << endl;
}
