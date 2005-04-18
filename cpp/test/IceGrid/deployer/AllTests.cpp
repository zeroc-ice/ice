// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <Ice/BuiltinSequences.h>
#include <Ice/IdentityUtil.h>
#include <IceGrid/Query.h>
#include <IceGrid/Admin.h>
#include <TestCommon.h>
#include <Test.h>

using namespace std;
using namespace Test;

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
    IceGrid::AdminPrx admin = IceGrid::AdminPrx::checkedCast(communicator->stringToProxy("IceGrid/Admin"));
    test(admin);

    cout << "test server registration... "  << flush;
    Ice::StringSeq serverNames = admin->getAllServerNames();
    test(find(serverNames.begin(), serverNames.end(), "Server1") != serverNames.end());
    test(find(serverNames.begin(), serverNames.end(), "Server2") != serverNames.end());
    test(find(serverNames.begin(), serverNames.end(), "IceBox1") != serverNames.end());
    test(find(serverNames.begin(), serverNames.end(), "IceBox2") != serverNames.end());
    cout << "ok" << endl;

    cout << "testing adapter registration... " << flush;
    Ice::StringSeq adapterIds = admin->getAllAdapterIds();
    test(find(adapterIds.begin(), adapterIds.end(), "Server1.Server") != adapterIds.end());
    test(find(adapterIds.begin(), adapterIds.end(), "Server2.Server") != adapterIds.end());
    test(find(adapterIds.begin(), adapterIds.end(), "IceBox1.Service1.Service1") != adapterIds.end());
    test(find(adapterIds.begin(), adapterIds.end(), "IceBox1Service2Adapter") != adapterIds.end());
    test(find(adapterIds.begin(), adapterIds.end(), "IceBox2.Service1.Service1") != adapterIds.end());
    test(find(adapterIds.begin(), adapterIds.end(), "IceBox2Service2Adapter") != adapterIds.end());
    cout << "ok" << endl;

    IceGrid::QueryPrx query = IceGrid::QueryPrx::checkedCast(communicator->stringToProxy("IceGrid/Query"));
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
    catch(const IceGrid::ObjectNotExistException&)
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

    TestIntfPrx obj;

    obj = TestIntfPrx::checkedCast(communicator->stringToProxy("Server1@Server1.Server"));
    obj = TestIntfPrx::checkedCast(communicator->stringToProxy("Server2@Server2.Server"));
    obj = TestIntfPrx::checkedCast(communicator->stringToProxy("IceBox1-Service1@IceBox1.Service1.Service1"));
    obj = TestIntfPrx::checkedCast(communicator->stringToProxy("IceBox1-Service2@IceBox1Service2Adapter"));
    obj = TestIntfPrx::checkedCast(communicator->stringToProxy("IceBox2-Service1@IceBox2.Service1.Service1"));
    obj = TestIntfPrx::checkedCast(communicator->stringToProxy("IceBox2-Service2@IceBox2Service2Adapter"));
    
    cout << "ok" << endl;

    cout << "testing server configuration... " << flush;

    obj = TestIntfPrx::checkedCast(communicator->stringToProxy("Server1@Server1.Server"));
    test(obj->getProperty("Type") == "Server");
    test(obj->getProperty("Name") == "Server1");

    test(obj->getProperty("Variable") == "val0prop");
    test(obj->getProperty("Variable1") == "");
    test(obj->getProperty("Variable2") == "");

    test(obj->getProperty("NameName") == "Server1Server1");
    test(obj->getProperty("NameEscaped") == "${name}");
    test(obj->getProperty("NameEscapeEscaped") == "$Server1");
    test(obj->getProperty("NameEscapedEscapeEscaped") == "$${name}");
    test(obj->getProperty("ManyEscape") == "$$$${name}");

    obj = TestIntfPrx::checkedCast(communicator->stringToProxy("Server2@Server2.Server"));
    test(obj->getProperty("Target1") == "1");
    test(obj->getProperty("Target2") == "1");
    test(obj->getProperty("Variable") == "val0prop");
    test(obj->getProperty("Variable1") == "val0target1");
    test(obj->getProperty("Variable2") == "val0target2");

    cout << "ok" << endl;

    cout << "testing service configuration... " << flush;

    obj = TestIntfPrx::checkedCast(communicator->stringToProxy("IceBox1-Service1@IceBox1.Service1.Service1"));
    test(obj->getProperty("Service1.Type") == "standard");
    test(obj->getProperty("Service1.ServiceName") == "Service1");
    
    test(obj->getProperty("Service1.InheritedVariable") == "inherited");

    obj = TestIntfPrx::checkedCast(communicator->stringToProxy("IceBox2-Service2@IceBox2Service2Adapter"));
    test(obj->getProperty("Service2.Type") == "freeze");
    test(obj->getProperty("Service2.ServiceName") == "Service2");

    test(obj->getProperty("Service2.DebugProperty") == "");
    test(obj->getProperty("Service1.DebugProperty") == "");
    
    IceGrid::AdminPrx admin = IceGrid::AdminPrx::checkedCast(communicator->stringToProxy("IceGrid/Admin"));
    test(admin);

    cout << "ok" << endl;

    cout << "testing server options... " << flush;

    obj = TestIntfPrx::checkedCast(communicator->stringToProxy("Server1@Server1.Server"));
    test(obj->getProperty("Test.Test") == "2");
    test(obj->getProperty("Test.Test1") == "0");

    cout << "ok" << endl;
}

void
allTestsWithTarget(const Ice::CommunicatorPtr& communicator)
{
    allCommonTests(communicator);

    IceGrid::AdminPrx admin = IceGrid::AdminPrx::checkedCast(
	communicator->stringToProxy("IceGrid/Admin"));
    test(admin);

    cout << "pinging server objects... " << flush;

    TestIntfPrx obj;
    admin->setServerActivation("Server1", IceGrid::Manual);
    try
    {
	obj = TestIntfPrx::checkedCast(communicator->stringToProxy("Server1@Server1.Server"));
	test(false);
    }
    catch(const Ice::LocalException&)
    {
    }
    admin->startServer("Server1");
    
    obj = TestIntfPrx::checkedCast(communicator->stringToProxy("Server1@Server1.Server"));
    test(obj->getProperty("Mode") == "manual");
    obj = TestIntfPrx::checkedCast(communicator->stringToProxy("Server2@Server2.Server"));

    cout << "ok" << endl;

    cout << "testing service configuration... " << flush;

    obj = TestIntfPrx::checkedCast(communicator->stringToProxy("IceBox1-Service1@IceBox1.Service1.Service1"));
    test(obj->getProperty("Service1.DebugProperty") == "debug");

    cout << "ok" << endl;
}
