// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <Ice/BuiltinSequences.h>
#include <Ice/Locator.h>
#include <IceGrid/IceGrid.h>
#include <IceUtil/Thread.h>
#include <TestCommon.h>
#include <Test.h>

using namespace std;
using namespace Test;
using namespace IceGrid;

namespace
{

const int sleepTime = 100; // 100ms
const int maxRetry = 240000 / sleepTime; // 4 minutes

void
addProperty(const CommunicatorDescriptorPtr& communicator, const string& name, const string& value)
{
    PropertyDescriptor prop;
    prop.name = name;
    prop.value = value;
    communicator->propertySet.properties.push_back(prop);
}

void
waitForServerState(const IceGrid::AdminPrx& admin, const std::string& server, bool up)
{
    int nRetry = 0;
    while(nRetry < maxRetry) // One minute
    {
        if(admin->getServerState(server) == (up ? Active : Inactive))
        {
            return;
        }

        IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(sleepTime));
        ++nRetry;
    }
    test(false);
}

void
waitForReplicaState(const IceGrid::AdminPrx& admin, const std::string& replica, bool up)
{
    int nRetry = 0;
    while(nRetry < maxRetry)
    {
        try
        {
            if(admin->pingRegistry(replica) == up)
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

        IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(sleepTime));
        ++nRetry;
    }

    try
    {
        if(admin->pingRegistry(replica) != up)
        {
            cerr << "replica state change timed out:" << endl;
            cerr << "replica: " << replica << endl;
            cerr << "state: " << up << endl;
        }
    }
    catch(const RegistryNotExistException&)
    {
        if(up)
        {
            cerr << "replica state change timed out:" << endl;
            cerr << "replica: " << replica << endl;
            cerr << "state: " << up << endl;
        }
    }

}

void
waitForNodeState(const IceGrid::AdminPrx& admin, const std::string& node, bool up)
{
    int nRetry = 0;
    while(nRetry < maxRetry)
    {
        try
        {
            if(admin->pingNode(node) == up) // Wait for the node to be removed.
            {
                return;
            }
        }
        catch(const NodeNotExistException&)
        {
            if(!up)
            {
                return;
            }
        }

        IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(sleepTime));
        ++nRetry;
    }
    try
    {
        if(admin->pingNode(node) != up)
        {
            cerr << "node state change timed out:" << endl;
            cerr << "node: " << node << endl;
            cerr << "state: " << up << endl;
        }
    }
    catch(const NodeNotExistException&)
    {
        if(up)
        {
            cerr << "node state change timed out:" << endl;
            cerr << "node: " << node << endl;
            cerr << "state: " << up << endl;
        }
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
    catch(const DeploymentException& ex)
    {
        cerr << ex.reason << endl;
        test(false);
    }
    catch(const Ice::LocalException& ex)
    {
        cerr << ex << endl;
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
    catch(const DeploymentException& ex)
    {
        cerr << ex.reason << endl;
        test(false);
    }
}

bool
waitAndPing(const Ice::ObjectPrx& obj)
{
    int nRetry = 0;
    while(nRetry < maxRetry)
    {
        try
        {
            obj->ice_ping();
            return true;
        }
        catch(const Ice::LocalException&)
        {
            IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(sleepTime));
            ++nRetry;
        }
    }
    return false;
}

AdminPrx
createAdminSession(const Ice::LocatorPrx& locator, const string& replica)
{
    test(waitAndPing(locator));

    string registryStr("RepTestIceGrid/Registry");
    if(!replica.empty() && replica != "Master")
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

}

void
allTests(const Ice::CommunicatorPtr& comm)
{
    IceGrid::RegistryPrx registry = IceGrid::RegistryPrx::checkedCast(
        comm->stringToProxy(comm->getDefaultLocator()->ice_getIdentity().category + "/Registry"));

    AdminSessionPrx session = registry->createAdminSession("foo", "bar");

    session->ice_getConnection()->setACM(registry->getACMTimeout(), IceUtil::None, Ice::HeartbeatAlways);

    AdminPrx admin = session->getAdmin();
    test(admin);

    map<string, string> params;

    params.clear();
    params["id"] = "Master";
    params["replicaName"] = "";
    params["port"] = "12050";
    instantiateServer(admin, "IceGridRegistry", params);

    params.clear();
    params["id"] = "Slave1";
    params["replicaName"] = "Slave1";
    params["port"] = "12051";
    instantiateServer(admin, "IceGridRegistry", params);

    params.clear();
    params["id"] = "Slave2";
    params["replicaName"] = "Slave2";
    params["port"] = "12052";
    instantiateServer(admin, "IceGridRegistry", params);

    Ice::LocatorPrx masterLocator =
        Ice::LocatorPrx::uncheckedCast(comm->stringToProxy("RepTestIceGrid/Locator-Master:default -p 12050"));
    Ice::LocatorPrx slave1Locator =
        Ice::LocatorPrx::uncheckedCast(comm->stringToProxy("RepTestIceGrid/Locator-Slave1:default -p 12051"));
    Ice::LocatorPrx slave2Locator =
        Ice::LocatorPrx::uncheckedCast(comm->stringToProxy("RepTestIceGrid/Locator-Slave2:default -p 12052"));

    Ice::LocatorPrx replicatedLocator =
        Ice::LocatorPrx::uncheckedCast(comm->stringToProxy("RepTestIceGrid/Locator:default -p 12050:default -p 12051"));

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

        info = masterAdmin->getObjectInfo(Ice::stringToIdentity("RepTestIceGrid/Locator"));
        ObjectInfo info1 = slave1Admin->getObjectInfo(Ice::stringToIdentity("RepTestIceGrid/Locator"));
        test(slave1Admin->getObjectInfo(Ice::stringToIdentity("RepTestIceGrid/Locator")) == info);
        test(info.type == Ice::Locator::ice_staticId());
        endpoints = info.proxy->ice_getEndpoints();
        test(endpoints.size() == 2);
        test(endpoints[0]->toString().find("-p 12050") != string::npos);
        test(endpoints[1]->toString().find("-p 12051") != string::npos);

        info = masterAdmin->getObjectInfo(Ice::stringToIdentity("RepTestIceGrid/Query"));
        test(slave1Admin->getObjectInfo(Ice::stringToIdentity("RepTestIceGrid/Query")) == info);
        test(info.type == IceGrid::Query::ice_staticId());
        endpoints = info.proxy->ice_getEndpoints();
        test(endpoints.size() == 2);
        test(endpoints[0]->toString().find("-p 12050") != string::npos);
        test(endpoints[1]->toString().find("-p 12051") != string::npos);

        admin->startServer("Slave2");
        slave2Admin = createAdminSession(slave2Locator, "Slave2");

        info = masterAdmin->getObjectInfo(Ice::stringToIdentity("RepTestIceGrid/Locator"));
        // We eventually need to wait here for the update of the replicated objects to propagate to the replica.
        int nRetry = 0;
        while(slave1Admin->getObjectInfo(Ice::stringToIdentity("RepTestIceGrid/Locator")) != info && nRetry < maxRetry)
        {
            IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(sleepTime));
            ++nRetry;
        }
        test(slave2Admin->getObjectInfo(Ice::stringToIdentity("RepTestIceGrid/Locator")) == info);
        test(info.type == Ice::Locator::ice_staticId());
        endpoints = info.proxy->ice_getEndpoints();
        test(endpoints.size() == 3);
        test(endpoints[0]->toString().find("-p 12050") != string::npos);
        test(endpoints[1]->toString().find("-p 12051") != string::npos);
        test(endpoints[2]->toString().find("-p 12052") != string::npos);

        info = masterAdmin->getObjectInfo(Ice::stringToIdentity("RepTestIceGrid/Query"));
        // We eventually need to wait here for the update of the replicated objects to propagate to the replica.
        nRetry = 0;
        while(slave1Admin->getObjectInfo(Ice::stringToIdentity("RepTestIceGrid/Query")) != info && nRetry < maxRetry)
        {
            IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(sleepTime));
            ++nRetry;
        }
        test(slave2Admin->getObjectInfo(Ice::stringToIdentity("RepTestIceGrid/Query")) == info);
        test(info.type == IceGrid::Query::ice_staticId());
        endpoints = info.proxy->ice_getEndpoints();
        test(endpoints.size() == 3);
        test(endpoints[0]->toString().find("-p 12050") != string::npos);
        test(endpoints[1]->toString().find("-p 12051") != string::npos);
        test(endpoints[2]->toString().find("-p 12052") != string::npos);

        slave2Admin->shutdown();
        waitForServerState(admin, "Slave2", false);

        info = masterAdmin->getObjectInfo(Ice::stringToIdentity("RepTestIceGrid/Locator"));
        // We eventually need to wait here for the update of the replicated objects to propagate to the replica.
        nRetry = 0;
        while(slave1Admin->getObjectInfo(Ice::stringToIdentity("RepTestIceGrid/Locator")) != info && nRetry < maxRetry)
        {
            IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(sleepTime));
            ++nRetry;
        }
        test(slave1Admin->getObjectInfo(Ice::stringToIdentity("RepTestIceGrid/Locator")) == info);
        test(info.type == Ice::Locator::ice_staticId());
        endpoints = info.proxy->ice_getEndpoints();
        test(endpoints.size() == 2);
        test(endpoints[0]->toString().find("-p 12050") != string::npos);
        test(endpoints[1]->toString().find("-p 12051") != string::npos);

        info = masterAdmin->getObjectInfo(Ice::stringToIdentity("RepTestIceGrid/Query"));
        nRetry = 0;
        while(slave1Admin->getObjectInfo(Ice::stringToIdentity("RepTestIceGrid/Query")) != info && nRetry < maxRetry)
        {
            IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(sleepTime));
            ++nRetry;
        }
        test(slave1Admin->getObjectInfo(Ice::stringToIdentity("RepTestIceGrid/Query")) == info);
        test(info.type == IceGrid::Query::ice_staticId());
        endpoints = info.proxy->ice_getEndpoints();
        test(endpoints.size() == 2);
        test(endpoints[0]->toString().find("-p 12050") != string::npos);
        test(endpoints[1]->toString().find("-p 12051") != string::npos);

        QueryPrx query;
        query = QueryPrx::uncheckedCast(comm->stringToProxy("RepTestIceGrid/Query:" + endpoints[0]->toString()));
        Ice::ObjectProxySeq objs = query->findAllObjectsByType("::IceGrid::Registry");
        test(objs.size() == 2);
        query = QueryPrx::uncheckedCast(comm->stringToProxy("RepTestIceGrid/Query:" + endpoints[1]->toString()));
        test(objs == query->findAllObjectsByType("::IceGrid::Registry"));
    }
    cout << "ok" << endl;

    cout << "testing well-known IceGrid objects... " << flush;
    {
        //
        // Test Registry well-known object (we have already tested
        // admin session creation for the creation of the admin
        // session above!)
        //
        RegistryPrx masterRegistry = RegistryPrx::checkedCast(
            comm->stringToProxy("RepTestIceGrid/Registry")->ice_locator(replicatedLocator));
        RegistryPrx slave1Registry = RegistryPrx::checkedCast(
            comm->stringToProxy("RepTestIceGrid/Registry-Slave1")->ice_locator(replicatedLocator));

        SessionPrx session = masterRegistry->createSession("dummy", "dummy");
        session->destroy();
        if(comm->getProperties()->getProperty("Ice.Default.Protocol") == "ssl")
        {
            session = masterRegistry->createSessionFromSecureConnection();
            session->destroy();
        }
        else
        {
            try
            {
                masterRegistry->createSessionFromSecureConnection();
            }
            catch(const PermissionDeniedException&)
            {
            }
        }

        try
        {
            slave1Registry->createSession("dummy", "");
        }
        catch(const PermissionDeniedException&)
        {
        }
        try
        {
            slave1Registry->createSessionFromSecureConnection();
        }
        catch(const PermissionDeniedException&)
        {
        }

        //
        // Test registry user-account mapper.
        //
        UserAccountMapperPrx masterMapper = UserAccountMapperPrx::checkedCast(
            comm->stringToProxy("RepTestIceGrid/RegistryUserAccountMapper")->ice_locator(replicatedLocator));
        UserAccountMapperPrx slave1Mapper = UserAccountMapperPrx::checkedCast(
            comm->stringToProxy("RepTestIceGrid/RegistryUserAccountMapper-Slave1")->ice_locator(replicatedLocator));

        test(masterMapper->getUserAccount("Dummy User Account1") == "dummy1");
        test(masterMapper->getUserAccount("Dummy User Account2") == "dummy2");
        test(slave1Mapper->getUserAccount("Dummy User Account1") == "dummy1");
        test(slave1Mapper->getUserAccount("Dummy User Account2") == "dummy2");
        try
        {
            masterMapper->getUserAccount("unknown");
            test(false);
        }
        catch(const UserAccountNotFoundException&)
        {
        }
        try
        {
            slave1Mapper->getUserAccount("unknown");
            test(false);
        }
        catch(const UserAccountNotFoundException&)
        {
        }

        //
        // Test SessionManager, SSLSessionManager,
        // AdminSessionManager, AdminSSLSessionManager
        //
        comm->stringToProxy("RepTestIceGrid/SessionManager")->ice_locator(replicatedLocator)->ice_ping();
        comm->stringToProxy("RepTestIceGrid/SSLSessionManager")->ice_locator(replicatedLocator)->ice_ping();
        try
        {
            comm->stringToProxy("RepTestIceGrid/SessionManager-Slave1")->ice_locator(replicatedLocator)->ice_ping();
            test(false);
        }
        catch(const Ice::NotRegisteredException&)
        {
        }
        try
        {
            comm->stringToProxy("RepTestIceGrid/SSLSessionManager-Slave1")->ice_locator(replicatedLocator)->ice_ping();
            test(false);
        }
        catch(const Ice::NotRegisteredException&)
        {
        }

        comm->stringToProxy("RepTestIceGrid/AdminSessionManager")->ice_locator(replicatedLocator)->ice_ping();
        comm->stringToProxy("RepTestIceGrid/AdminSSLSessionManager")->ice_locator(replicatedLocator)->ice_ping();
        comm->stringToProxy("RepTestIceGrid/AdminSessionManager-Slave1")->ice_locator(replicatedLocator)->ice_ping();
        comm->stringToProxy("RepTestIceGrid/AdminSSLSessionManager-Slave1")->ice_locator(replicatedLocator)->ice_ping();
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
        waitForServerState(admin, "Slave2", false);

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
        waitForServerState(admin, "Slave2", false);

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
        waitForServerState(admin, "Slave2", false);

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
        waitForServerState(admin, "Slave2", false);
    }
    cout << "ok" << endl;

    params.clear();
    params["id"] = "Node1";
    instantiateServer(admin, "IceGridNode", params);

    //
    // Add an application which is using Node1. Otherwise, when a
    // registry restarts it would throw aways the proxy of the nodes
    // because the node isn't used by any application.
    //
    ApplicationDescriptor app;
    app.name = "DummyApp";
    app.nodes["Node1"].description = "dummy node";
    try
    {
        masterAdmin->addApplication(app);
    }
    catch(const Ice::Exception& ex)
    {
        cerr << ex << endl;
        test(false);
    }

    //
    // Test node session establishment.
    //
    // - start master, start slave1, start node, start slave2
    // - shutdown slave1, start slave1 -> node should re-connect
    // - shutdown master
    // - shutdown slave2, start slave2 -> node should re-connect
    // - shutdown slave1
    // - start master -> node connects to master
    // - start slave1 -> node connects to slave1
    //
    cout << "testing node session establishment... " << flush;
    {
        admin->startServer("Node1");

        waitForNodeState(masterAdmin, "Node1", true);
        waitForNodeState(slave1Admin, "Node1", true);

        admin->startServer("Slave2");
        slave2Admin = createAdminSession(slave2Locator, "Slave2");

        waitForNodeState(slave2Admin, "Node1", true); // Node should connect.

        slave1Admin->shutdown();
        waitForServerState(admin, "Slave1", false);
        admin->startServer("Slave1");
        slave1Admin = createAdminSession(slave1Locator, "Slave1");

        try
        {
            test(slave1Admin->pingNode("Node1")); // Node should be re-connected.
        }
        catch(const NodeNotExistException&)
        {
            test(false);
        }

        masterAdmin->shutdown();
        waitForServerState(admin, "Master", false);

        slave2Admin->shutdown();
        waitForServerState(admin, "Slave2", false);
        admin->startServer("Slave2");
        slave2Admin = createAdminSession(slave2Locator, "Slave2");

        try
        {
            //
            // On slow environments, it can take a bit for the node to
            // re-establish the connection so we ping it twice. The
            // second should succeed.
            //
            slave2Admin->pingNode("Node1");
            test(slave2Admin->pingNode("Node1")); // Node should be re-connected even if the master is down.
        }
        catch(const NodeNotExistException&)
        {
            test(false);
        }

        slave1Admin->shutdown();
        waitForServerState(admin, "Slave1", false);

        admin->startServer("Master");
        masterAdmin = createAdminSession(masterLocator, "");

        try
        {
            test(masterAdmin->pingNode("Node1")); // Node should be re-connected.
        }
        catch(const NodeNotExistException&)
        {
            test(false);
        }

        admin->startServer("Slave1");
        slave1Admin = createAdminSession(slave1Locator, "Slave1");

        try
        {
            test(slave1Admin->pingNode("Node1")); // Node should be re-connected.
        }
        catch(const NodeNotExistException&)
        {
            test(false);
        }

        try
        {
            test(masterAdmin->pingNode("Node1"));
        }
        catch(const NodeNotExistException&)
        {
            test(false);
        }

        try
        {
            test(slave2Admin->pingNode("Node1"));
        }
        catch(const NodeNotExistException&)
        {
            test(false);
        }

        slave2Admin->shutdown();
        waitForServerState(admin, "Slave2", false);
        admin->startServer("Slave2");
        slave2Admin = createAdminSession(slave2Locator, "Slave2");
        try
        {
            test(slave2Admin->pingNode("Node1"));
        }
        catch(const NodeNotExistException&)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    //
    // Testing updates with out-of-date replicas.
    //
    cout << "testing out-of-date replicas... " << flush;
    {
        ApplicationDescriptor app;
        app.name = "TestApp";
        app.description = "added application";

        ServerDescriptorPtr server = new ServerDescriptor();
        server->id = "Server";
        server->exe = comm->getProperties()->getProperty("ServerDir") + "/server";
        server->pwd = ".";
        server->applicationDistrib = false;
        server->allocatable = false;
        addProperty(server, "Ice.Admin.Endpoints", "tcp -h 127.0.0.1");
        server->activation = "on-demand";
        AdapterDescriptor adapter;
        adapter.name = "TestAdapter";
        adapter.id = "TestAdapter.Server";
        adapter.registerProcess = false;
        adapter.serverLifetime = true;
        PropertyDescriptor property;
        property.name = "TestAdapter.Endpoints";
        property.value = "default";
        server->propertySet.properties.push_back(property);
        property.name = "Identity";
        property.value = "test";
        server->propertySet.properties.push_back(property);
        ObjectDescriptor object;
        object.id = Ice::stringToIdentity("test");
        object.type = "::Test::TestIntf";
        adapter.objects.push_back(object);
        server->adapters.push_back(adapter);
        app.nodes["Node1"].servers.push_back(server);

        masterAdmin->addApplication(app);

        try
        {
            comm->stringToProxy("test")->ice_locator(masterLocator)->ice_locatorCacheTimeout(0)->ice_ping();
            comm->stringToProxy("test")->ice_locator(slave1Locator)->ice_locatorCacheTimeout(0)->ice_ping();
            comm->stringToProxy("test")->ice_locator(slave2Locator)->ice_locatorCacheTimeout(0)->ice_ping();
        }
        catch(const Ice::LocalException& ex)
        {
            cerr << ex << endl;

            ApplicationInfo app = admin->getApplicationInfo("Test");
            cerr << "properties-override = " << app.descriptor.variables["properties-override"] << endl;

            PropertyDescriptorSeq& seq = admin->getServerInfo("Node1").descriptor->propertySet.properties;
            for(PropertyDescriptorSeq::const_iterator p = seq.begin(); p != seq.end(); ++p)
            {
                cerr << p->name << " = " << p->value << endl;
            }
            test(false);
        }

        masterAdmin->stopServer("Server");

        //
        // Shutdown Slave2 and update application.
        //
        slave2Admin->shutdown();
        waitForServerState(admin, "Slave2", false);

        ApplicationUpdateDescriptor update;
        update.name = "TestApp";
        NodeUpdateDescriptor node;
        node.name = "Node1";
        node.servers.push_back(server);
        update.nodes.push_back(node);
        property.name = "Dummy";
        property.value = "val";
        server->propertySet.properties.push_back(property);
        masterAdmin->updateApplication(update);

        try
        {
            comm->stringToProxy("test")->ice_locator(masterLocator)->ice_locatorCacheTimeout(0)->ice_ping();
            comm->stringToProxy("test")->ice_locator(slave1Locator)->ice_locatorCacheTimeout(0)->ice_ping();
        }
        catch(const Ice::LocalException& ex)
        {
            cerr << ex << endl;
            test(false);
        }

        masterAdmin->shutdown();
        waitForServerState(admin, "Master", false);

        admin->startServer("Slave2");
        slave2Admin = createAdminSession(slave2Locator, "Slave2");
        waitForNodeState(slave2Admin, "Node1", true); // Node should connect.

        try
        {
            slave2Admin->startServer("Server");
            test(false);
        }
        catch(const DeploymentException&)
        {
        }
        try
        {
            comm->stringToProxy("test")->ice_locator(slave2Locator)->ice_locatorCacheTimeout(0)->ice_ping();
            test(false);
        }
        catch(const Ice::NoEndpointException&)
        {
        }

        admin->startServer("Master");
        masterAdmin = createAdminSession(masterLocator, "");

        slave2Admin->shutdown();
        waitForServerState(admin, "Slave2", false);
        admin->startServer("Slave2");
        slave2Admin = createAdminSession(slave2Locator, "Slave2");

        try
        {
            comm->stringToProxy("test")->ice_locator(slave2Locator)->ice_locatorCacheTimeout(0)->ice_ping();
        }
        catch(const Ice::LocalException& ex)
        {
            cerr << ex << endl;
            test(false);
        }

        //
        // Shutdown Node1 and update the application, then, shutdown
        // the master.
        //
        slave1Admin->shutdownNode("Node1");
        waitForServerState(admin, "Node1", false);

        slave2Admin->shutdown();
        waitForServerState(admin, "Slave2", false);

        property.name = "Dummy2";
        property.value = "val";
        server->propertySet.properties.push_back(property);
        masterAdmin->updateApplication(update);

        masterAdmin->shutdown();
        waitForServerState(admin, "Master", false);

        //
        // Restart Node1 and Slave2, Slave2 still has the old version
        // of the server so it should be able to load it. Slave1 has
        // a more recent version, so it can't load it.
        //
        admin->startServer("Slave2");
        slave2Admin = createAdminSession(slave2Locator, "Slave2");

        admin->startServer("Node1");

        waitForNodeState(slave2Admin, "Node1", true);

        slave1Admin->shutdown();
        waitForServerState(admin, "Slave1", false);

        try
        {
            comm->stringToProxy("test")->ice_locator(slave2Locator)->ice_locatorCacheTimeout(0)->ice_ping();
        }
        catch(const Ice::LocalException& ex)
        {
            cerr << ex << endl;
            test(false);
        }

        admin->startServer("Slave1");
        slave1Admin = createAdminSession(slave1Locator, "Slave1");

        try
        {
            comm->stringToProxy("test")->ice_locator(slave1Locator)->ice_locatorCacheTimeout(0)->ice_ping();
        }
        catch(const Ice::NoEndpointException&)
        {
        }

        try
        {
            comm->stringToProxy("test")->ice_locator(slave2Locator)->ice_locatorCacheTimeout(0)->ice_ping();
        }
        catch(const Ice::LocalException& ex)
        {
            cerr << ex << endl;
            test(false);
        }
        slave2Admin->stopServer("Server");

        //
        // Start the master. This will re-load the server on the node
        // and update the out-of-date replicas.
        //
        admin->startServer("Master");
        masterAdmin = createAdminSession(masterLocator, "");

        slave1Admin->shutdown();
        waitForServerState(admin, "Slave1", false);
        admin->startServer("Slave1");
        slave1Admin = createAdminSession(slave1Locator, "Slave1");

        slave2Admin->shutdownNode("Node1");
        waitForServerState(admin, "Node1", false);
        admin->startServer("Node1");

        slave2Admin->shutdown();
        waitForServerState(admin, "Slave2", false);
        admin->startServer("Slave2");
        slave2Admin = createAdminSession(slave2Locator, "Slave2");

        waitForNodeState(masterAdmin, "Node1", true);
        waitForNodeState(slave1Admin, "Node1", true);
        waitForNodeState(slave2Admin, "Node1", true);

        try
        {
            comm->stringToProxy("test")->ice_locator(masterLocator)->ice_locatorCacheTimeout(0)->ice_ping();
            comm->stringToProxy("test")->ice_locator(slave1Locator)->ice_locatorCacheTimeout(0)->ice_ping();
            comm->stringToProxy("test")->ice_locator(slave2Locator)->ice_locatorCacheTimeout(0)->ice_ping();
        }
        catch(const Ice::LocalException& ex)
        {
            cerr << ex << endl;
            test(false);
        }

        slave2Admin->stopServer("Server");

        masterAdmin->removeApplication("TestApp");
    }
    cout << "ok" << endl;

    cout << "testing master upgrade... " << flush;
    {
        ApplicationDescriptor app;
        app.name = "TestApp";
        app.description = "added application";

        ServerDescriptorPtr server = new ServerDescriptor();
        server->id = "Server";
        server->exe = comm->getProperties()->getProperty("ServerDir") + "/server";
        server->pwd = ".";
        server->applicationDistrib = false;
        server->allocatable = false;
        addProperty(server, "Ice.Admin.Endpoints", "tcp -h 127.0.0.1");
        server->activation = "on-demand";
        AdapterDescriptor adapter;
        adapter.name = "TestAdapter";
        adapter.id = "TestAdapter.Server";
        adapter.serverLifetime = true;
        adapter.registerProcess = false;
        PropertyDescriptor property;
        property.name = "TestAdapter.Endpoints";
        property.value = "default";
        server->propertySet.properties.push_back(property);
        property.name = "Identity";
        property.value = "test";
        server->propertySet.properties.push_back(property);
        ObjectDescriptor object;
        object.id = Ice::stringToIdentity("test");
        object.type = "::Test::TestIntf";
        adapter.objects.push_back(object);
        server->adapters.push_back(adapter);
        app.nodes["Node1"].servers.push_back(server);

        masterAdmin->addApplication(app);

        comm->stringToProxy("test")->ice_locator(masterLocator)->ice_locatorCacheTimeout(0)->ice_ping();
        comm->stringToProxy("test")->ice_locator(slave1Locator)->ice_locatorCacheTimeout(0)->ice_ping();
        comm->stringToProxy("test")->ice_locator(slave2Locator)->ice_locatorCacheTimeout(0)->ice_ping();
        masterAdmin->stopServer("Server");

        //
        // Shutdown the Master, update Slave1 to be the Master.
        //
        masterAdmin->shutdown();
        waitForServerState(admin, "Master", false);
        slave1Admin->shutdown();
        waitForServerState(admin, "Slave1", false);

        params.clear();
        params["id"] = "Slave1";
        params["port"] = "12051";
        params["replicaName"] = "Master";
        instantiateServer(admin, "IceGridRegistry", params);

        admin->startServer("Slave1");
        slave1Locator =
            Ice::LocatorPrx::uncheckedCast(comm->stringToProxy("RepTestIceGrid/Locator-Master:default -p 12051"));
        slave1Admin = createAdminSession(slave1Locator, "");

        waitForReplicaState(slave1Admin, "Slave2", true);

        ApplicationUpdateDescriptor update;
        update.name = "TestApp";
        NodeUpdateDescriptor node;
        node.name = "Node1";
        node.servers.push_back(server);
        update.nodes.push_back(node);
        property.name = "Dummy";
        property.value = "val";
        server->propertySet.properties.push_back(property);
        slave1Admin->updateApplication(update);

        comm->stringToProxy("test")->ice_locator(slave1Locator)->ice_locatorCacheTimeout(0)->ice_ping();
        comm->stringToProxy("test")->ice_locator(slave2Locator)->ice_locatorCacheTimeout(0)->ice_ping();

        slave1Admin->shutdown();
        waitForServerState(admin, "Slave1", false);

        params.clear();
        params["id"] = "Slave1";
        params["replicaName"] = "Slave1";
        params["port"] = "12051";
        instantiateServer(admin, "IceGridRegistry", params);

        params.clear();
        params["id"] = "Master";
        params["replicaName"] = "";
        params["port"] = "12050";
        params["arg"] = "--initdb-from-replica=Slave2";
        instantiateServer(admin, "IceGridRegistry", params);

        admin->startServer("Master");
        masterAdmin = createAdminSession(masterLocator, "");

        admin->startServer("Slave1");
        slave1Locator =
            Ice::LocatorPrx::uncheckedCast(comm->stringToProxy("RepTestIceGrid/Locator-Slave1:default -p 12051"));
        slave1Admin = createAdminSession(slave1Locator, "Slave1");

        comm->stringToProxy("test")->ice_locator(masterLocator)->ice_locatorCacheTimeout(0)->ice_ping();
        comm->stringToProxy("test")->ice_locator(slave1Locator)->ice_locatorCacheTimeout(0)->ice_ping();
        comm->stringToProxy("test")->ice_locator(slave2Locator)->ice_locatorCacheTimeout(0)->ice_ping();

        masterAdmin->stopServer("Server");

        waitForReplicaState(masterAdmin, "Slave1", true);
        waitForReplicaState(masterAdmin, "Slave2", true);

        ApplicationInfo info = masterAdmin->getApplicationInfo("TestApp");
        test(info.revision == 2);

        masterAdmin->removeApplication("TestApp");
    }
    cout << "ok" << endl;

    cout << "testing interop with registry and node using the 1.0 encoding... " << flush;
    {
        params.clear();
        params["id"] = "Slave3";
        params["replicaName"] = "Slave3";
        params["port"] = "12053";
        params["encoding"] = "1.0";
        instantiateServer(admin, "IceGridRegistry", params);

        params.clear();
        params["id"] = "Node2";
        params["encoding"] = "1.0";
        instantiateServer(admin, "IceGridNode", params);

        admin->startServer("Slave3");
        waitForServerState(admin, "Slave3", true);
        waitForReplicaState(masterAdmin, "Slave3", true);

        admin->startServer("Node2");
        waitForNodeState(masterAdmin, "Node2", true);

        Ice::LocatorPrx slave3Locator =
            Ice::LocatorPrx::uncheckedCast(
                comm->stringToProxy("RepTestIceGrid/Locator-Slave3 -e 1.0:default -p 12053"));
        IceGrid::AdminPrx slave3Admin = createAdminSession(slave3Locator, "Slave3");
        waitForNodeState(slave3Admin, "Node2", true);

        ApplicationDescriptor app;
        app.name = "TestApp";
        app.description = "added application";

        ServerDescriptorPtr server = new ServerDescriptor();
        server->id = "Server";
        server->exe = comm->getProperties()->getProperty("ServerDir") + "/server";
        server->pwd = ".";
        server->applicationDistrib = false;
        server->allocatable = false;
        addProperty(server, "Ice.Admin.Endpoints", "tcp -h 127.0.0.1");
        server->activation = "on-demand";
        AdapterDescriptor adapter;
        adapter.name = "TestAdapter";
        adapter.id = "TestAdapter.Server";
        adapter.serverLifetime = true;
        adapter.registerProcess = false;
        PropertyDescriptor property;
        property.name = "TestAdapter.Endpoints";
        property.value = "default";
        server->propertySet.properties.push_back(property);
        property.name = "Identity";
        property.value = "test";
        server->propertySet.properties.push_back(property);
        ObjectDescriptor object;
        object.id = Ice::stringToIdentity("test");
        object.type = "::Test::TestIntf";
        adapter.objects.push_back(object);
        server->adapters.push_back(adapter);
        app.nodes["Node2"].servers.push_back(server);

        masterAdmin->addApplication(app);

        comm->stringToProxy("test -e 1.0")->ice_locator(
            masterLocator->ice_encodingVersion(Ice::Encoding_1_0))->ice_locatorCacheTimeout(0)->ice_ping();
        comm->stringToProxy("test -e 1.0")->ice_locator(
            slave1Locator->ice_encodingVersion(Ice::Encoding_1_0))->ice_locatorCacheTimeout(0)->ice_ping();
        comm->stringToProxy("test -e 1.0")->ice_locator(slave3Locator)->ice_locatorCacheTimeout(0)->ice_ping();
        masterAdmin->stopServer("Server");

    }
    cout << "ok" << endl;


    slave1Admin->shutdownNode("Node1");
    removeServer(admin, "Node1");

    removeServer(admin, "Slave2");

    slave1Admin->shutdown();
    removeServer(admin, "Slave1");
    masterAdmin->shutdown();
    removeServer(admin, "Master");
}
