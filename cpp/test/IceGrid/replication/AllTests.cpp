// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <Ice/BuiltinSequences.h>
#include <Ice/IdentityUtil.h>
#include <Ice/Locator.h>
#include <IceGrid/Query.h>
#include <IceGrid/Registry.h>
#include <IceGrid/Admin.h>
#include <TestCommon.h>
#include <Test.h>

using namespace std;
using namespace Test;
using namespace IceGrid;

class SessionKeepAliveThread : public IceUtil::Thread, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    SessionKeepAliveThread(const IceGrid::AdminSessionPrx& session, long timeout) :
	_session(session),
        _timeout(IceUtil::Time::seconds(timeout)),
        _destroy(false)
    {
    }

    virtual void
    run()
    {
        Lock sync(*this);
        while(!_destroy)
        {
            timedWait(_timeout);
            if(_destroy)
            {
	        break;
	    }
            try
            {
                _session->keepAlive();
            }
            catch(const Ice::Exception&)
            {
		break;
            }
        }
    }

    void
    destroy()
    {
        Lock sync(*this);
        _destroy = true;
        notify();
    }

private:

    IceGrid::AdminSessionPrx _session;
    const IceUtil::Time _timeout;
    bool _destroy;
};
typedef IceUtil::Handle<SessionKeepAliveThread> SessionKeepAliveThreadPtr;

void
waitForRegistryState(const IceGrid::AdminPrx& admin, const std::string& registry, bool up)
{
    int nRetry = 0;
    while(nRetry < 15)
    {
	try
	{
	    if(admin->pingRegistry(registry) && up) // Wait for the registry to be removed.
	    {
		return;
	    }
	}
	catch(const RegistryNotExistException&)
	{
	    if(!up)
	    {
		return;
	    }
	}
	
	IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(500));
	++nRetry;
    }
    if(admin->pingRegistry(registry) != up)
    {
	cerr << "registry state change timed out:" << endl;
	cerr << "registry: " << registry << endl;
	cerr << "state: " << up << endl;
    }
}

void
instantiateServer(const AdminPrx& admin, const string& templ, const map<string, string>& params)
{
    ServerInstanceDescriptor desc;
    desc._cpp_template = templ;
    desc.parameterValues = params;
    NodeUpdateDescriptor nodeUpdate;
    nodeUpdate.name = "localnode";
    nodeUpdate.serverInstances.push_back(desc);
    ApplicationUpdateDescriptor update;
    update.name = "Test";
    update.nodes.push_back(nodeUpdate);
    try
    {
	admin->updateApplication(update);
    }
    catch(DeploymentException& ex)
    {
	cerr << ex.reason << endl;
	test(false);
    }
}

void
removeServer(const AdminPrx& admin, const string& id)
{
    try
    {
	admin->stopServer(id);
    }
    catch(const ServerStopException&)
    {
    }
    catch(const NodeUnreachableException&)
    {
    }
    catch(const Ice::UserException& ex)
    {
	cerr << ex << endl;
	test(false);
    }

    NodeUpdateDescriptor nodeUpdate;
    nodeUpdate.name = "localnode";
    nodeUpdate.removeServers.push_back(id);
    ApplicationUpdateDescriptor update;
    update.name = "Test";
    update.nodes.push_back(nodeUpdate);
    try
    {
	admin->updateApplication(update);
    }
    catch(DeploymentException& ex)
    {
	cerr << ex.reason << endl;
	test(false);
    }
}

bool
waitAndPing(const Ice::ObjectPrx& obj)
{
    int nRetry = 0;
    while(nRetry < 20)
    {
	try
	{
	    obj->ice_ping();
	    return true;
	}
	catch(const Ice::LocalException&)
	{
	    IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(500));
	    ++nRetry;
	}
    }
    return false;
}

AdminPrx
createAdminSession(const Ice::LocatorPrx& locator, const string& replica)
{
    test(waitAndPing(locator));
    
    string registryStr("TestIceGrid/Registry");
    if(!replica.empty())
    {
	registryStr += "-" + replica;
    }
    Ice::ObjectPrx obj = locator->ice_getCommunicator()->stringToProxy(registryStr)->ice_locator(locator);
    RegistryPrx registry = RegistryPrx::checkedCast(obj);
    test(registry);

    AdminSessionPrx session = AdminSessionPrx::checkedCast(registry->createAdminSession("foo", "bar"));
    test(session);
    return session->getAdmin();
}

void
allTests(const Ice::CommunicatorPtr& comm)
{
    RegistryPrx registry = IceGrid::RegistryPrx::checkedCast(comm->stringToProxy("IceGrid/Registry"));
    test(registry);
    AdminSessionPrx session = registry->createAdminSession("foo", "bar");

    SessionKeepAliveThreadPtr keepAlive = new SessionKeepAliveThread(session, registry->getSessionTimeout() / 2);
    keepAlive->start();

    AdminPrx admin = session->getAdmin();
    test(admin);

    map<string, string> params;

    params.clear();
    params["id"] = "Master";
    params["port"] = "12050";
    instantiateServer(admin, "IceGridRegistry", params);
    
    params.clear();
    params["id"] = "Slave1";
    params["port"] = "12051";
    instantiateServer(admin, "IceGridRegistry", params);
    
    params.clear();
    params["id"] = "Slave2";
    params["port"] = "12052";
    instantiateServer(admin, "IceGridRegistry", params);

    Ice::LocatorPrx masterLocator = 
	Ice::LocatorPrx::uncheckedCast(comm->stringToProxy("TestIceGrid/Locator:default -p 12050"));
    Ice::LocatorPrx slave1Locator = 
	Ice::LocatorPrx::uncheckedCast(comm->stringToProxy("TestIceGrid/Locator:default -p 12051"));
    Ice::LocatorPrx slave2Locator = 
	Ice::LocatorPrx::uncheckedCast(comm->stringToProxy("TestIceGrid/Locator:default -p 12052"));

    AdminPrx masterAdmin, slave1Admin, slave2Admin;

    admin->startServer("Master");
    masterAdmin = createAdminSession(masterLocator, "");
    
    admin->startServer("Slave1");
    slave1Admin = createAdminSession(slave1Locator, "Slave1");

    //
    // Test replication and well-known objects:
    //
    // - Locator interface
    // - Query interface
    //
    // - Registry object
    // - RegistryUserAccountMapper
    // - SessionManager/SSLSessionManager
    // - AdminSessionManager/AdminSSLSessionManager
    //
    cout << "testing replicated locator and query interface... " << flush;
    {
	Ice::EndpointSeq endpoints;
	ObjectInfo info;
	
	info = masterAdmin->getObjectInfo(comm->stringToIdentity("TestIceGrid/Locator"));
	ObjectInfo info1 = slave1Admin->getObjectInfo(comm->stringToIdentity("TestIceGrid/Locator"));
	test(slave1Admin->getObjectInfo(comm->stringToIdentity("TestIceGrid/Locator")) == info);
	test(info.type == Ice::Locator::ice_staticId());
	endpoints = info.proxy->ice_getEndpoints();
	test(endpoints.size() == 2);
	test(endpoints[0]->toString() == masterLocator->ice_getEndpoints()[0]->toString());
	test(endpoints[1]->toString() == slave1Locator->ice_getEndpoints()[0]->toString());

	info = masterAdmin->getObjectInfo(comm->stringToIdentity("TestIceGrid/Query"));
	test(slave1Admin->getObjectInfo(comm->stringToIdentity("TestIceGrid/Query")) == info);
	test(info.type == IceGrid::Query::ice_staticId());
	endpoints = info.proxy->ice_getEndpoints();
	test(endpoints.size() == 2);
	test(endpoints[0]->toString() == masterLocator->ice_getEndpoints()[0]->toString());
	test(endpoints[1]->toString() == slave1Locator->ice_getEndpoints()[0]->toString());

	admin->startServer("Slave2");
	slave2Admin = createAdminSession(slave2Locator, "Slave2");

	info = masterAdmin->getObjectInfo(comm->stringToIdentity("TestIceGrid/Locator"));
	test(slave1Admin->getObjectInfo(comm->stringToIdentity("TestIceGrid/Locator")) == info);
	test(slave2Admin->getObjectInfo(comm->stringToIdentity("TestIceGrid/Locator")) == info);
	test(info.type == Ice::Locator::ice_staticId());
	endpoints = info.proxy->ice_getEndpoints();
	test(endpoints.size() == 3);
	test(endpoints[0]->toString() == masterLocator->ice_getEndpoints()[0]->toString());
	test(endpoints[1]->toString() == slave1Locator->ice_getEndpoints()[0]->toString());
	test(endpoints[2]->toString() == slave2Locator->ice_getEndpoints()[0]->toString());

	info = masterAdmin->getObjectInfo(comm->stringToIdentity("TestIceGrid/Query"));
	test(slave1Admin->getObjectInfo(comm->stringToIdentity("TestIceGrid/Query")) == info);
	test(slave2Admin->getObjectInfo(comm->stringToIdentity("TestIceGrid/Query")) == info);
	test(info.type == IceGrid::Query::ice_staticId());
	endpoints = info.proxy->ice_getEndpoints();
	test(endpoints.size() == 3);
	test(endpoints[0]->toString() == masterLocator->ice_getEndpoints()[0]->toString());
	test(endpoints[1]->toString() == slave1Locator->ice_getEndpoints()[0]->toString());
	test(endpoints[2]->toString() == slave2Locator->ice_getEndpoints()[0]->toString());

	slave2Admin->shutdown();
	waitForRegistryState(admin, "Slave2", false);

	info = masterAdmin->getObjectInfo(comm->stringToIdentity("TestIceGrid/Locator"));
	test(slave1Admin->getObjectInfo(comm->stringToIdentity("TestIceGrid/Locator")) == info);
	test(info.type == Ice::Locator::ice_staticId());
	endpoints = info.proxy->ice_getEndpoints();
	test(endpoints.size() == 2);
	test(endpoints[0]->toString() == masterLocator->ice_getEndpoints()[0]->toString());
	test(endpoints[1]->toString() == slave1Locator->ice_getEndpoints()[0]->toString());

	info = masterAdmin->getObjectInfo(comm->stringToIdentity("TestIceGrid/Query"));
	test(slave1Admin->getObjectInfo(comm->stringToIdentity("TestIceGrid/Query")) == info);
	test(info.type == IceGrid::Query::ice_staticId());
	endpoints = info.proxy->ice_getEndpoints();
	test(endpoints.size() == 2);
	test(endpoints[0]->toString() == masterLocator->ice_getEndpoints()[0]->toString());
	test(endpoints[1]->toString() == slave1Locator->ice_getEndpoints()[0]->toString());
    }
    cout << "ok" << endl;

    cout << "testing replicated query interface... " << flush;
    {
    }
    cout << "ok" << endl;

    cout << "testing well-known IceGrid objects... " << flush;
    {
    }
    cout << "ok" << endl;

    //
    // Registry update test:
    //
    // - start master
    // - start slave1: keep slave1 up for each update
    // - start slave2: shutdown slave2 for each update
    // - ensure updates are correctly replicated
    // - updates to test: application/adapter/object
    //
    cout << "testing registry updates... " << flush;
    {
	ApplicationDescriptor app;
	app.name = "TestApp";
	app.description = "added application";

	AdapterInfo adpt;
	adpt.id = "TestAdpt";
	adpt.proxy = comm->stringToProxy("dummy:tcp -p 12345 -h 127.0.0.1");

	ObjectInfo obj;
	obj.proxy = comm->stringToProxy("dummy:tcp -p 12345 -h 127.0.0.1");
	obj.type = "::Hello";

	//
	// We use the locator registry from Slave1 to ensure that the
	// forwarding to the master work (the slave locator registry
	// forwards everything to the master).
	//
	Ice::LocatorRegistryPrx locatorRegistry = slave1Locator->getRegistry();

	//
	// Test addition of application, adapter, object.
	//
	
	try
	{
	    slave1Admin->addApplication(app);
	    test(false);
	}
	catch(const DeploymentException&)
	{
	    // Slave can't modify the database.
	}
	masterAdmin->addApplication(app);

	locatorRegistry->setAdapterDirectProxy(adpt.id, adpt.proxy);

	try
	{
	    slave1Admin->addObjectWithType(obj.proxy, obj.type);
	    test(false);
	}
	catch(const DeploymentException&)
	{
	    // Slave can't modify the database
	}
	masterAdmin->addObjectWithType(obj.proxy, obj.type);

	admin->startServer("Slave2");
	slave2Admin = createAdminSession(slave2Locator, "Slave2");

	test(masterAdmin->getApplicationInfo("TestApp").descriptor.description == "added application");
	test(slave1Admin->getApplicationInfo("TestApp").descriptor.description == "added application");
	test(slave2Admin->getApplicationInfo("TestApp").descriptor.description == "added application");	
	    
	test(masterAdmin->getAdapterInfo("TestAdpt")[0] == adpt);
	test(slave1Admin->getAdapterInfo("TestAdpt")[0] == adpt);
	test(slave2Admin->getAdapterInfo("TestAdpt")[0] == adpt);

	test(masterAdmin->getObjectInfo(obj.proxy->ice_getIdentity()) == obj);
	test(slave1Admin->getObjectInfo(obj.proxy->ice_getIdentity()) == obj);
	test(slave2Admin->getObjectInfo(obj.proxy->ice_getIdentity()) == obj);
	
	slave2Admin->shutdown();
	waitForRegistryState(admin, "Slave2", false);

	//
	// Test sync of application.
	//

	app.description = "updated1 application";
	try
	{
	    slave1Admin->syncApplication(app);
	    test(false);
	}
	catch(const DeploymentException&)
	{
	    // Slave can't modify the database.
	}
	masterAdmin->syncApplication(app);

	admin->startServer("Slave2");
	slave2Admin = createAdminSession(slave2Locator, "Slave2");
	test(masterAdmin->getApplicationInfo("TestApp").descriptor.description == "updated1 application");
	test(slave1Admin->getApplicationInfo("TestApp").descriptor.description == "updated1 application");
	test(slave2Admin->getApplicationInfo("TestApp").descriptor.description == "updated1 application");
	slave2Admin->shutdown();
	waitForRegistryState(admin, "Slave2", false);

	//
	// Test update of application, adapter, object.
	//

	ApplicationUpdateDescriptor appUpdate;
	appUpdate.name = "TestApp";
	appUpdate.description = new BoxedString("updated2 application");
	try
	{
	    slave1Admin->updateApplication(appUpdate);
	    test(false);
	}
	catch(const DeploymentException&)
	{
	    // Slave can't modify the database.
	}
	masterAdmin->updateApplication(appUpdate);

	adpt.replicaGroupId = "TestReplicaGroup";
	locatorRegistry->setReplicatedAdapterDirectProxy(adpt.id, adpt.replicaGroupId, adpt.proxy);

	obj.proxy = comm->stringToProxy("dummy:tcp -p 12346 -h 127.0.0.1");
	try
	{
	    slave1Admin->updateObject(obj.proxy);
	    test(false);
	}
	catch(const DeploymentException&)
	{
	    // Slave can't modify the database
	}
	masterAdmin->updateObject(obj.proxy);

	admin->startServer("Slave2");
	slave2Admin = createAdminSession(slave2Locator, "Slave2");

	test(masterAdmin->getApplicationInfo("TestApp").descriptor.description == "updated2 application");
	test(slave1Admin->getApplicationInfo("TestApp").descriptor.description == "updated2 application");
	test(slave2Admin->getApplicationInfo("TestApp").descriptor.description == "updated2 application");

	test(masterAdmin->getAdapterInfo("TestAdpt")[0] == adpt);
	test(slave1Admin->getAdapterInfo("TestAdpt")[0] == adpt);
	test(slave2Admin->getAdapterInfo("TestAdpt")[0] == adpt);

	test(masterAdmin->getObjectInfo(obj.proxy->ice_getIdentity()) == obj);
	test(slave1Admin->getObjectInfo(obj.proxy->ice_getIdentity()) == obj);
	test(slave2Admin->getObjectInfo(obj.proxy->ice_getIdentity()) == obj);
	
	slave2Admin->shutdown();
	waitForRegistryState(admin, "Slave2", false);

	//
	// Test removal of application, adapter and object.

	try
	{
	    slave1Admin->removeApplication("TestApp");
	    test(false);
	}
	catch(const DeploymentException&)
	{
	    // Slave can't modify the database.
	}
	masterAdmin->removeApplication("TestApp");

	try
	{
	    slave1Admin->removeAdapter("TestAdpt");
	    test(false);
	}
	catch(const DeploymentException&)
	{
	    // Slave can't modify the database.
	}
	masterAdmin->removeAdapter("TestAdpt");
	try
	{
	    slave1Admin->removeObject(obj.proxy->ice_getIdentity());
	}
	catch(const DeploymentException&)
	{
	    // Slave can't modify the database.
	}
	masterAdmin->removeObject(obj.proxy->ice_getIdentity());

	admin->startServer("Slave2");
	slave2Admin = createAdminSession(slave2Locator, "Slave2");
	try
	{
	    masterAdmin->getApplicationInfo("TestApp");
	    test(false);
	}
	catch(const ApplicationNotExistException&)
	{
	}
	try
	{
	    slave1Admin->getApplicationInfo("TestApp");
	    test(false);
	}
	catch(const ApplicationNotExistException&)
	{
	}
	try
	{
	    slave2Admin->getApplicationInfo("TestApp");
	    test(false);
	}
	catch(const ApplicationNotExistException&)
	{
	}
	try
	{
	    masterAdmin->getAdapterInfo("TestAdpt");
	    test(false);
	}
	catch(const AdapterNotExistException&)
	{
	}
	try
	{
	    slave1Admin->getAdapterInfo("TestAdpt");
	    test(false);
	}
	catch(const AdapterNotExistException&)
	{
	}
	try
	{
	    slave2Admin->getAdapterInfo("TestAdpt");
	    test(false);
	}
	catch(const AdapterNotExistException&)
	{
	}
	try
	{
	    masterAdmin->getObjectInfo(obj.proxy->ice_getIdentity());
	    test(false);
	}
	catch(const ObjectNotRegisteredException&)
	{
	}
	try
	{
	    slave1Admin->getObjectInfo(obj.proxy->ice_getIdentity());
	    test(false);
	}
	catch(const ObjectNotRegisteredException&)
	{
	}
	try
	{
	    slave2Admin->getObjectInfo(obj.proxy->ice_getIdentity());
	    test(false);
	}
	catch(const ObjectNotRegisteredException&)
	{
	}

	slave2Admin->shutdown();
	waitForRegistryState(admin, "Slave2", false);
    }
    cout << "ok" << endl;

    removeServer(admin, "Slave2");
    slave1Admin->shutdown();
    removeServer(admin, "Slave1");
    masterAdmin->shutdown();
    removeServer(admin, "Master");

    //
    // Test node session establishment.
    //
    // - start master, start slave1, start node, start slave2
    // - shutdown slave1, start slave1 -> node should re-connect
    // - shutdown master
    // - shutdown slave2, start slave2 -> node should re-connect
    // - start slave3 -> node can't connect to it
    // - shutdown slave1
    // - start master -> node connects to master, slave3
    // - start slave1 -> node connects to slave1
    //
}
