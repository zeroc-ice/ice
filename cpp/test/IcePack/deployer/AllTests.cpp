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
#include <Ice/BuiltinSequences.h>
#include <Ice/IdentityUtil.h>
#include <IcePack/Admin.h>
#include <Yellow/Yellow.h>
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
    IcePack::AdminPrx admin = IcePack::AdminPrx::checkedCast(
	communicator->stringToProxy("IcePack/Admin@IcePack.Registry.Admin"));
    test(admin);

    cout << "test server registration..." << flush;
    Ice::StringSeq serverNames = admin->getAllServerNames();
    test(find(serverNames.begin(), serverNames.end(), "Server1") != serverNames.end());
    test(find(serverNames.begin(), serverNames.end(), "Server2") != serverNames.end());
    test(find(serverNames.begin(), serverNames.end(), "IceBox1") != serverNames.end());
    test(find(serverNames.begin(), serverNames.end(), "IceBox2") != serverNames.end());
    cout << "ok" << endl;

    cout << "testing adapter registration... " << flush;
    Ice::StringSeq adapterNames = admin->getAllAdapterNames();
    test(find(adapterNames.begin(), adapterNames.end(), "Server-Server1") != adapterNames.end());
    test(find(adapterNames.begin(), adapterNames.end(), "Server-Server2") != adapterNames.end());
    test(find(adapterNames.begin(), adapterNames.end(), "Service1-IceBox1.Service1") != adapterNames.end());
    test(find(adapterNames.begin(), adapterNames.end(), "IceBox1Service2Adapter") != adapterNames.end());
    test(find(adapterNames.begin(), adapterNames.end(), "Service1-IceBox2.Service1") != adapterNames.end());
    test(find(adapterNames.begin(), adapterNames.end(), "IceBox2Service2Adapter") != adapterNames.end());
    cout << "ok" << endl;

    Yellow::QueryPrx yellow = Yellow::QueryPrx::checkedCast(
	communicator->stringToProxy("Yellow/Query@Yellow.Query"));
    test(yellow);

    cout << "testing offer registration... " << flush;
    Ice::ObjectProxySeq offers = yellow->lookupAll("::Test");
    test(find_if(offers.begin(), offers.end(), bind2nd(ProxyIdentityEqual(),"Server1")) != offers.end());
    test(find_if(offers.begin(), offers.end(), bind2nd(ProxyIdentityEqual(),"Server2")) != offers.end());
    test(find_if(offers.begin(), offers.end(), bind2nd(ProxyIdentityEqual(),"IceBox1-Service1")) != offers.end());
    test(find_if(offers.begin(), offers.end(), bind2nd(ProxyIdentityEqual(),"IceBox1-Service2")) != offers.end());
    test(find_if(offers.begin(), offers.end(), bind2nd(ProxyIdentityEqual(),"IceBox2-Service1")) != offers.end());
    test(find_if(offers.begin(), offers.end(), bind2nd(ProxyIdentityEqual(),"IceBox2-Service2")) != offers.end());

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
    
    cout << "ok" << endl;
}

void
allTestsWithTarget(const Ice::CommunicatorPtr& communicator)
{
    allCommonTests(communicator);

    IcePack::AdminPrx admin = IcePack::AdminPrx::checkedCast(
	communicator->stringToProxy("IcePack/Admin@IcePack.Registry.Admin"));
    test(admin);

    cout << "pinging server objects... " << flush;

    //
    // Application is deployed with Server1.manual which disables on demand activation.
    //
    TestPrx obj;
    try
    {
	obj = TestPrx::checkedCast(communicator->stringToProxy("Server1@Server-Server1"));
	test(false);
    }
    catch(const Ice::RuntimeException&)
    {
    }
    admin->startServer("Server1");
    
    obj = TestPrx::checkedCast(communicator->stringToProxy("Server2@Server-Server2"));

    cout << "ok" << endl;

    cout << "testing service configuration... " << flush;

    obj = TestPrx::checkedCast(communicator->stringToProxy("IceBox1-Service1@Service1-IceBox1.Service1"));
    test(obj->getProperty("Service1.DebugProperty") == "debug");

    cout << "ok" << endl;
}
