// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "Ice/Locator.h"
#include "IceGrid/IceGrid.h"
#include "TestHelper.h"
#include <thread>

using namespace std;
using namespace Test;
using namespace Ice;
using namespace IceGrid;

namespace
{
    // NOLINTBEGIN(cert-err58-cpp)
    const auto sleepTime = 100ms;
    const int maxRetry = static_cast<int>(120000 / sleepTime.count()); // 2 minutes
    // NOLINTEND(cert-err58-cpp)

    void waitForServerState(const optional<AdminPrx>& admin, const string& server, bool up)
    {
        int nRetry = 0;
        while (nRetry < maxRetry)
        {
            if (admin->getServerState(server) == (up ? ServerState::Active : ServerState::Inactive))
            {
                return;
            }

            this_thread::sleep_for(sleepTime);
            ++nRetry;
        }
        test(false);
    }

    void waitForReplicaState(const optional<AdminPrx>& admin, const string& replica, bool up)
    {
        int nRetry = 0;
        while (nRetry < maxRetry)
        {
            try
            {
                if (admin->pingRegistry(replica) == up)
                {
                    return;
                }
            }
            catch (const RegistryNotExistException&)
            {
                if (!up)
                {
                    return;
                }
            }

            this_thread::sleep_for(sleepTime);
            ++nRetry;
        }

        try
        {
            if (admin->pingRegistry(replica) != up)
            {
                cerr << "replica state change timed out:" << endl;
                cerr << "replica: " << replica << endl;
                cerr << "state: " << up << endl;
            }
        }
        catch (const RegistryNotExistException&)
        {
            if (up)
            {
                cerr << "replica state change timed out:" << endl;
                cerr << "replica: " << replica << endl;
                cerr << "state: " << up << endl;
            }
        }
    }

    void waitForNodeState(const optional<AdminPrx>& admin, const string& node, bool up)
    {
        int nRetry = 0;
        while (nRetry < maxRetry)
        {
            try
            {
                if (admin->pingNode(node) == up) // Wait for the node to be removed.
                {
                    return;
                }
            }
            catch (const NodeNotExistException&)
            {
                if (!up)
                {
                    return;
                }
            }

            this_thread::sleep_for(sleepTime);
            ++nRetry;
        }
        try
        {
            if (admin->pingNode(node) != up)
            {
                cerr << "node state change timed out:" << endl;
                cerr << "node: " << node << endl;
                cerr << "state: " << up << endl;
            }
        }
        catch (const NodeNotExistException&)
        {
            if (up)
            {
                cerr << "node state change timed out:" << endl;
                cerr << "node: " << node << endl;
                cerr << "state: " << up << endl;
            }
        }
    }

    void instantiateServer(const optional<AdminPrx>& admin, string templ, const map<string, string>& params)
    {
        ServerInstanceDescriptor desc;
        desc.templateName = std::move(templ);
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
        catch (const DeploymentException& ex)
        {
            cerr << ex.reason << endl;
            test(false);
        }
        catch (const Ice::LocalException& ex)
        {
            cerr << ex << endl;
            test(false);
        }
    }

    void removeServer(const optional<AdminPrx>& admin, const string& id)
    {
        try
        {
            admin->stopServer(id);
        }
        catch (const ServerStopException&)
        {
        }
        catch (const NodeUnreachableException&)
        {
        }
        catch (const Ice::UserException& ex)
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
        catch (const DeploymentException& ex)
        {
            cerr << ex.reason << endl;
            test(false);
        }
    }

    bool waitAndPing(const Ice::ObjectPrx& obj)
    {
        int nRetry = 0;
        while (nRetry < maxRetry)
        {
            try
            {
                obj->ice_ping();
                return true;
            }
            catch (const Ice::LocalException&)
            {
                this_thread::sleep_for(sleepTime);
                ++nRetry;
            }
        }
        return false;
    }

    optional<AdminPrx> createAdminSession(const Ice::LocatorPrx& locator, const string& replica)
    {
        test(waitAndPing(locator));

        string registryStr("RepTestIceGrid/Registry");
        if (!replica.empty() && replica != "Master")
        {
            registryStr += "-" + replica;
        }

        auto registry = RegistryPrx(locator->ice_getCommunicator(), registryStr)->ice_locator(locator);

        optional<AdminSessionPrx> session = registry->createAdminSession("foo", "bar");
        test(session);
        return session->getAdmin();
    }

    bool isObjectInfoEqual(const ObjectInfo& info1, const ObjectInfo& info2)
    {
        return (info1.type == info2.type) && (info1.proxy == info2.proxy);
    }

    bool isAdapterInfoEqual(const AdapterInfo& adpt1, const AdapterInfo& adpt2)
    {
        return (adpt1.id == adpt2.id) && (adpt1.replicaGroupId == adpt2.replicaGroupId) && (adpt1.proxy == adpt2.proxy);
    }
}

void
allTests(Test::TestHelper* helper)
{
    auto communicator = helper->communicator();
    IceGrid::RegistryPrx registry(
        communicator,
        communicator->getDefaultLocator()->ice_getIdentity().category + "/Registry");

    auto adminSession = registry->createAdminSession("foo", "bar");
    auto admin = adminSession->getAdmin();
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

    Ice::LocatorPrx masterLocator(communicator, "RepTestIceGrid/Locator-Master:default -p 12050");

    Ice::LocatorPrx slave1Locator(communicator, "RepTestIceGrid/Locator-Slave1:default -p 12051");

    Ice::LocatorPrx slave2Locator(communicator, "RepTestIceGrid/Locator-Slave2:default -p 12052");

    Ice::LocatorPrx replicatedLocator(communicator, "RepTestIceGrid/Locator:default -p 12050:default -p 12051");

    admin->startServer("Master");
    optional<AdminPrx> masterAdmin = createAdminSession(masterLocator, "");

    admin->startServer("Slave1");
    optional<AdminPrx> slave1Admin = createAdminSession(slave1Locator, "Slave1");

    optional<AdminPrx> slave2Admin;

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
        auto info1 = slave1Admin->getObjectInfo(Ice::stringToIdentity("RepTestIceGrid/Locator"));
        test(isObjectInfoEqual(slave1Admin->getObjectInfo(Ice::stringToIdentity("RepTestIceGrid/Locator")), info));
        test(info.type == Ice::Locator::ice_staticId());
        endpoints = info.proxy->ice_getEndpoints();
        test(endpoints.size() == 2);
        test(endpoints[0]->toString().find("-p 12050") != string::npos);
        test(endpoints[1]->toString().find("-p 12051") != string::npos);
        info = masterAdmin->getObjectInfo(Ice::stringToIdentity("RepTestIceGrid/Query"));
        test(isObjectInfoEqual(slave1Admin->getObjectInfo(Ice::stringToIdentity("RepTestIceGrid/Query")), info));
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
        while (!isObjectInfoEqual(slave1Admin->getObjectInfo(Ice::stringToIdentity("RepTestIceGrid/Locator")), info) &&
               nRetry < maxRetry)
        {
            this_thread::sleep_for(sleepTime);
            ++nRetry;
        }
        test(isObjectInfoEqual(slave2Admin->getObjectInfo(Ice::stringToIdentity("RepTestIceGrid/Locator")), info));
        test(info.type == Ice::Locator::ice_staticId());
        endpoints = info.proxy->ice_getEndpoints();
        test(endpoints.size() == 3);
        test(endpoints[0]->toString().find("-p 12050") != string::npos);
        test(endpoints[1]->toString().find("-p 12051") != string::npos);
        test(endpoints[2]->toString().find("-p 12052") != string::npos);
        info = masterAdmin->getObjectInfo(Ice::stringToIdentity("RepTestIceGrid/Query"));
        // We eventually need to wait here for the update of the replicated objects to propagate to the replica.
        nRetry = 0;
        while (!isObjectInfoEqual(slave1Admin->getObjectInfo(Ice::stringToIdentity("RepTestIceGrid/Query")), info) &&
               nRetry < maxRetry)
        {
            this_thread::sleep_for(sleepTime);
            ++nRetry;
        }
        test(isObjectInfoEqual(slave2Admin->getObjectInfo(Ice::stringToIdentity("RepTestIceGrid/Query")), info));
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
        while (!isObjectInfoEqual(slave1Admin->getObjectInfo(Ice::stringToIdentity("RepTestIceGrid/Locator")), info) &&
               nRetry < maxRetry)
        {
            this_thread::sleep_for(sleepTime);
            ++nRetry;
        }
        test(isObjectInfoEqual(slave1Admin->getObjectInfo(Ice::stringToIdentity("RepTestIceGrid/Locator")), info));
        test(info.type == Ice::Locator::ice_staticId());
        endpoints = info.proxy->ice_getEndpoints();
        test(endpoints.size() == 2);
        test(endpoints[0]->toString().find("-p 12050") != string::npos);
        test(endpoints[1]->toString().find("-p 12051") != string::npos);

        info = masterAdmin->getObjectInfo(Ice::stringToIdentity("RepTestIceGrid/Query"));
        nRetry = 0;
        while (!isObjectInfoEqual(slave1Admin->getObjectInfo(Ice::stringToIdentity("RepTestIceGrid/Query")), info) &&
               nRetry < maxRetry)
        {
            this_thread::sleep_for(sleepTime);
            ++nRetry;
        }
        test(isObjectInfoEqual(slave1Admin->getObjectInfo(Ice::stringToIdentity("RepTestIceGrid/Query")), info));
        test(info.type == IceGrid::Query::ice_staticId());
        endpoints = info.proxy->ice_getEndpoints();
        test(endpoints.size() == 2);
        test(endpoints[0]->toString().find("-p 12050") != string::npos);
        test(endpoints[1]->toString().find("-p 12051") != string::npos);

        QueryPrx query(communicator, "RepTestIceGrid/Query:" + endpoints[0]->toString());
        auto objs1 = query->findAllObjectsByType("::IceGrid::Registry");
        test(objs1.size() == 2);

        query = QueryPrx(communicator, "RepTestIceGrid/Query:" + endpoints[1]->toString());
        auto objs2 = query->findAllObjectsByType("::IceGrid::Registry");
        for (vector<Ice::ObjectPrx>::size_type i = 0; i < objs1.size(); i++)
        {
            test(objs1[i] == objs2[i]);
        }
    }
    cout << "ok" << endl;

    cout << "testing well-known IceGrid objects... " << flush;
    {
        //
        // Test Registry well-known object (we have already tested
        // admin session creation for the creation of the admin
        // session above!)
        //
        auto masterRegistry = RegistryPrx(communicator, "RepTestIceGrid/Registry")->ice_locator(replicatedLocator);
        auto slave1Registry =
            RegistryPrx(communicator, "RepTestIceGrid/Registry-Slave1")->ice_locator(replicatedLocator);

        auto session = masterRegistry->createSession("dummy", "dummy");
        session->destroy();
        if (communicator->getProperties()->getIceProperty("Ice.Default.Protocol") == "ssl")
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
            catch (const PermissionDeniedException&)
            {
            }
        }

        try
        {
            slave1Registry->createSession("dummy", "");
        }
        catch (const PermissionDeniedException&)
        {
        }
        try
        {
            slave1Registry->createSessionFromSecureConnection();
        }
        catch (const PermissionDeniedException&)
        {
        }

        //
        // Test registry user-account mapper.
        //
        auto masterMapper = UserAccountMapperPrx(communicator, "RepTestIceGrid/RegistryUserAccountMapper")
                                ->ice_locator(replicatedLocator);

        auto slave1Mapper = UserAccountMapperPrx(communicator, "RepTestIceGrid/RegistryUserAccountMapper-Slave1")
                                ->ice_locator(replicatedLocator);

        test(masterMapper->getUserAccount("Dummy User Account1") == "dummy1");
        test(masterMapper->getUserAccount("Dummy User Account2") == "dummy2");
        test(slave1Mapper->getUserAccount("Dummy User Account1") == "dummy1");
        test(slave1Mapper->getUserAccount("Dummy User Account2") == "dummy2");
        try
        {
            masterMapper->getUserAccount("unknown");
            test(false);
        }
        catch (const UserAccountNotFoundException&)
        {
        }
        try
        {
            slave1Mapper->getUserAccount("unknown");
            test(false);
        }
        catch (const UserAccountNotFoundException&)
        {
        }

        //
        // Test SessionManager, SSLSessionManager,
        // AdminSessionManager, AdminSSLSessionManager
        //
        ObjectPrx(communicator, "RepTestIceGrid/SessionManager")->ice_locator(replicatedLocator)->ice_ping();
        ObjectPrx(communicator, "RepTestIceGrid/SSLSessionManager")->ice_locator(replicatedLocator)->ice_ping();
        try
        {
            ObjectPrx(communicator, "RepTestIceGrid/SessionManager-Slave1")->ice_locator(replicatedLocator)->ice_ping();
            test(false);
        }
        catch (const Ice::NotRegisteredException&)
        {
        }
        try
        {
            ObjectPrx(communicator, "RepTestIceGrid/SSLSessionManager-Slave1")
                ->ice_locator(replicatedLocator)
                ->ice_ping();
            test(false);
        }
        catch (const Ice::NotRegisteredException&)
        {
        }

        ObjectPrx(communicator, "RepTestIceGrid/AdminSessionManager")->ice_locator(replicatedLocator)->ice_ping();
        ObjectPrx(communicator, "RepTestIceGrid/AdminSSLSessionManager")->ice_locator(replicatedLocator)->ice_ping();
        ObjectPrx(communicator, "RepTestIceGrid/AdminSessionManager-Slave1")
            ->ice_locator(replicatedLocator)
            ->ice_ping();
        ObjectPrx(communicator, "RepTestIceGrid/AdminSSLSessionManager-Slave1")
            ->ice_locator(replicatedLocator)
            ->ice_ping();
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
        adpt.proxy = ObjectPrx(communicator, "dummy:tcp -p 12345 -h 127.0.0.1");

        ObjectInfo obj;
        obj.proxy = ObjectPrx(communicator, "dummy:tcp -p 12345 -h 127.0.0.1");
        obj.type = "::Hello";

        //
        // We use the locator registry from Slave1 to ensure that the
        // forwarding to the master work (the slave locator registry
        // forwards everything to the master).
        //
        auto locatorRegistry = slave1Locator->getRegistry();

        //
        // Test addition of application, adapter, object.
        //

        try
        {
            slave1Admin->addApplication(app);
            test(false);
        }
        catch (const DeploymentException&)
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
        catch (const DeploymentException&)
        {
            // Slave can't modify the database
        }
        masterAdmin->addObjectWithType(obj.proxy, obj.type);

        admin->startServer("Slave2");
        slave2Admin = createAdminSession(slave2Locator, "Slave2");

        test(masterAdmin->getApplicationInfo("TestApp").descriptor.description == "added application");
        test(slave1Admin->getApplicationInfo("TestApp").descriptor.description == "added application");
        test(slave2Admin->getApplicationInfo("TestApp").descriptor.description == "added application");

        test(isAdapterInfoEqual(masterAdmin->getAdapterInfo("TestAdpt")[0], adpt));
        test(isAdapterInfoEqual(slave1Admin->getAdapterInfo("TestAdpt")[0], adpt));
        test(isAdapterInfoEqual(slave2Admin->getAdapterInfo("TestAdpt")[0], adpt));

        test(isObjectInfoEqual(masterAdmin->getObjectInfo(obj.proxy->ice_getIdentity()), obj));
        test(isObjectInfoEqual(slave1Admin->getObjectInfo(obj.proxy->ice_getIdentity()), obj));
        test(isObjectInfoEqual(slave2Admin->getObjectInfo(obj.proxy->ice_getIdentity()), obj));

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
        catch (const DeploymentException&)
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
        appUpdate.description = make_shared<BoxedString>("updated2 application");
        try
        {
            slave1Admin->updateApplication(appUpdate);
            test(false);
        }
        catch (const DeploymentException&)
        {
            // Slave can't modify the database.
        }
        masterAdmin->updateApplication(appUpdate);

        adpt.replicaGroupId = "TestReplicaGroup";
        locatorRegistry->setReplicatedAdapterDirectProxy(adpt.id, adpt.replicaGroupId, adpt.proxy);

        obj.proxy = ObjectPrx(communicator, "dummy:tcp -p 12346 -h 127.0.0.1");
        try
        {
            slave1Admin->updateObject(obj.proxy);
            test(false);
        }
        catch (const DeploymentException&)
        {
            // Slave can't modify the database
        }
        masterAdmin->updateObject(obj.proxy);

        admin->startServer("Slave2");
        slave2Admin = createAdminSession(slave2Locator, "Slave2");

        test(masterAdmin->getApplicationInfo("TestApp").descriptor.description == "updated2 application");
        test(slave1Admin->getApplicationInfo("TestApp").descriptor.description == "updated2 application");
        test(slave2Admin->getApplicationInfo("TestApp").descriptor.description == "updated2 application");

        test(isAdapterInfoEqual(masterAdmin->getAdapterInfo("TestAdpt")[0], adpt));
        test(isAdapterInfoEqual(slave1Admin->getAdapterInfo("TestAdpt")[0], adpt));
        test(isAdapterInfoEqual(slave2Admin->getAdapterInfo("TestAdpt")[0], adpt));

        test(isObjectInfoEqual(masterAdmin->getObjectInfo(obj.proxy->ice_getIdentity()), obj));
        test(isObjectInfoEqual(slave1Admin->getObjectInfo(obj.proxy->ice_getIdentity()), obj));
        test(isObjectInfoEqual(slave2Admin->getObjectInfo(obj.proxy->ice_getIdentity()), obj));

        slave2Admin->shutdown();
        waitForServerState(admin, "Slave2", false);

        //
        // Test removal of application, adapter and object.

        try
        {
            slave1Admin->removeApplication("TestApp");
            test(false);
        }
        catch (const DeploymentException&)
        {
            // Slave can't modify the database.
        }
        masterAdmin->removeApplication("TestApp");

        try
        {
            slave1Admin->removeAdapter("TestAdpt");
            test(false);
        }
        catch (const DeploymentException&)
        {
            // Slave can't modify the database.
        }
        masterAdmin->removeAdapter("TestAdpt");
        try
        {
            slave1Admin->removeObject(obj.proxy->ice_getIdentity());
        }
        catch (const DeploymentException&)
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
        catch (const ApplicationNotExistException&)
        {
        }
        try
        {
            slave1Admin->getApplicationInfo("TestApp");
            test(false);
        }
        catch (const ApplicationNotExistException&)
        {
        }
        try
        {
            slave2Admin->getApplicationInfo("TestApp");
            test(false);
        }
        catch (const ApplicationNotExistException&)
        {
        }
        try
        {
            masterAdmin->getAdapterInfo("TestAdpt");
            test(false);
        }
        catch (const AdapterNotExistException&)
        {
        }
        try
        {
            slave1Admin->getAdapterInfo("TestAdpt");
            test(false);
        }
        catch (const AdapterNotExistException&)
        {
        }
        try
        {
            slave2Admin->getAdapterInfo("TestAdpt");
            test(false);
        }
        catch (const AdapterNotExistException&)
        {
        }
        try
        {
            masterAdmin->getObjectInfo(obj.proxy->ice_getIdentity());
            test(false);
        }
        catch (const ObjectNotRegisteredException&)
        {
        }
        try
        {
            slave1Admin->getObjectInfo(obj.proxy->ice_getIdentity());
            test(false);
        }
        catch (const ObjectNotRegisteredException&)
        {
        }
        try
        {
            slave2Admin->getObjectInfo(obj.proxy->ice_getIdentity());
            test(false);
        }
        catch (const ObjectNotRegisteredException&)
        {
        }

        slave2Admin->shutdown();
        waitForServerState(admin, "Slave2", false);
    }
    cout << "ok" << endl;

    params.clear();
    params["id"] = "Node1";
    instantiateServer(admin, "IceGridNode", params);

    {
        //
        // Add an application which is using Node1. Otherwise, when a
        // registry restarts it would throw away the proxy of the nodes
        // because the node isn't used by any application.
        //
        ApplicationDescriptor app;
        app.name = "DummyApp";
        app.nodes["Node1"].description = "dummy node";
        try
        {
            masterAdmin->addApplication(app);
        }
        catch (const Ice::Exception& ex)
        {
            cerr << ex << endl;
            test(false);
        }
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
            //
            // On slow environments, it can take a bit for the node to
            // re-establish the connection so we ping it twice. The
            // second should succeed.
            //
            if (!slave1Admin->pingNode("Node1"))
            {
                this_thread::sleep_for(200ms);
            }
            test(slave1Admin->pingNode("Node1")); // Node should be re-connected.
        }
        catch (const NodeNotExistException&)
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
            if (!slave2Admin->pingNode("Node1"))
            {
                this_thread::sleep_for(200ms);
            }
            test(slave2Admin->pingNode("Node1")); // Node should be re-connected even if the master is down.
        }
        catch (const NodeNotExistException&)
        {
            test(false);
        }

        slave1Admin->shutdown();
        waitForServerState(admin, "Slave1", false);

        admin->startServer("Master");
        masterAdmin = createAdminSession(masterLocator, "");

        try
        {
            if (!masterAdmin->pingNode("Node1"))
            {
                this_thread::sleep_for(200ms);
            }
            test(masterAdmin->pingNode("Node1")); // Node should be re-connected.
        }
        catch (const NodeNotExistException&)
        {
            test(false);
        }

        admin->startServer("Slave1");
        slave1Admin = createAdminSession(slave1Locator, "Slave1");

        try
        {
            if (!slave1Admin->pingNode("Node1"))
            {
                this_thread::sleep_for(200ms);
            }
            test(slave1Admin->pingNode("Node1")); // Node should be re-connected.
        }
        catch (const NodeNotExistException&)
        {
            test(false);
        }

        try
        {
            if (!masterAdmin->pingNode("Node1"))
            {
                this_thread::sleep_for(200ms);
            }
            test(masterAdmin->pingNode("Node1"));
        }
        catch (const NodeNotExistException&)
        {
            test(false);
        }

        try
        {
            if (!slave2Admin->pingNode("Node1"))
            {
                this_thread::sleep_for(200ms);
            }
            test(slave2Admin->pingNode("Node1"));
        }
        catch (const NodeNotExistException&)
        {
            test(false);
        }

        slave2Admin->shutdown();
        waitForServerState(admin, "Slave2", false);
        admin->startServer("Slave2");
        slave2Admin = createAdminSession(slave2Locator, "Slave2");
        try
        {
            if (!slave2Admin->pingNode("Node1"))
            {
                this_thread::sleep_for(200ms);
            }
            test(slave2Admin->pingNode("Node1"));
        }
        catch (const NodeNotExistException&)
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

        auto server = make_shared<ServerDescriptor>();
        server->id = "Server";
        server->exe = communicator->getProperties()->getProperty("ServerDir") + "/server";
        server->pwd = ".";
        server->allocatable = false;
        PropertyDescriptor prop{"Ice.Admin.Endpoints", "tcp -h 127.0.0.1"};
        server->propertySet.properties.push_back(prop);
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
            ObjectPrx(communicator, "test")->ice_locator(masterLocator)->ice_locatorCacheTimeout(0)->ice_ping();
            ObjectPrx(communicator, "test")->ice_locator(slave1Locator)->ice_locatorCacheTimeout(0)->ice_ping();
            ObjectPrx(communicator, "test")->ice_locator(slave2Locator)->ice_locatorCacheTimeout(0)->ice_ping();
        }
        catch (const Ice::LocalException& ex)
        {
            cerr << ex << endl;

            auto appInfo = admin->getApplicationInfo("Test");
            cerr << "properties-override = " << appInfo.descriptor.variables["properties-override"] << endl;

            auto propertySeq = admin->getServerInfo("Node1").descriptor->propertySet.properties;
            for (const auto& p : propertySeq)
            {
                cerr << p.name << " = " << p.value << endl;
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
            ObjectPrx(communicator, "test")->ice_locator(masterLocator)->ice_locatorCacheTimeout(0)->ice_ping();
            ObjectPrx(communicator, "test")->ice_locator(slave1Locator)->ice_locatorCacheTimeout(0)->ice_ping();
        }
        catch (const Ice::LocalException& ex)
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
        catch (const DeploymentException&)
        {
        }
        try
        {
            ObjectPrx(communicator, "test")->ice_locator(slave2Locator)->ice_locatorCacheTimeout(0)->ice_ping();
            test(false);
        }
        catch (const Ice::NoEndpointException&)
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
            ObjectPrx(communicator, "test")->ice_locator(slave2Locator)->ice_locatorCacheTimeout(0)->ice_ping();
        }
        catch (const Ice::LocalException& ex)
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
            ObjectPrx(communicator, "test")->ice_locator(slave2Locator)->ice_locatorCacheTimeout(0)->ice_ping();
        }
        catch (const Ice::LocalException& ex)
        {
            cerr << ex << endl;
            test(false);
        }

        admin->startServer("Slave1");
        slave1Admin = createAdminSession(slave1Locator, "Slave1");

        try
        {
            ObjectPrx(communicator, "test")->ice_locator(slave1Locator)->ice_locatorCacheTimeout(0)->ice_ping();
        }
        catch (const Ice::NoEndpointException&)
        {
        }

        try
        {
            ObjectPrx(communicator, "test")->ice_locator(slave2Locator)->ice_locatorCacheTimeout(0)->ice_ping();
        }
        catch (const Ice::LocalException& ex)
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
            ObjectPrx(communicator, "test")->ice_locator(masterLocator)->ice_locatorCacheTimeout(0)->ice_ping();
            ObjectPrx(communicator, "test")->ice_locator(slave1Locator)->ice_locatorCacheTimeout(0)->ice_ping();
            ObjectPrx(communicator, "test")->ice_locator(slave2Locator)->ice_locatorCacheTimeout(0)->ice_ping();
        }
        catch (const Ice::LocalException& ex)
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

        auto server = make_shared<ServerDescriptor>();
        server->id = "Server";
        server->exe = communicator->getProperties()->getProperty("ServerDir") + "/server";
        server->pwd = ".";
        server->allocatable = false;
        PropertyDescriptor prop{"Ice.Admin.Endpoints", "tcp -h 127.0.0.1"};
        server->propertySet.properties.push_back(prop);
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

        ObjectPrx(communicator, "test")->ice_locator(masterLocator)->ice_locatorCacheTimeout(0)->ice_ping();
        ObjectPrx(communicator, "test")->ice_locator(slave1Locator)->ice_locatorCacheTimeout(0)->ice_ping();
        ObjectPrx(communicator, "test")->ice_locator(slave2Locator)->ice_locatorCacheTimeout(0)->ice_ping();
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
        slave1Locator = Ice::LocatorPrx(communicator, "RepTestIceGrid/Locator-Master:default -p 12051");
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

        ObjectPrx(communicator, "test")->ice_locator(slave1Locator)->ice_locatorCacheTimeout(0)->ice_ping();
        ObjectPrx(communicator, "test")->ice_locator(slave2Locator)->ice_locatorCacheTimeout(0)->ice_ping();

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
        slave1Locator = Ice::LocatorPrx(communicator, "RepTestIceGrid/Locator-Slave1:default -p 12051");
        slave1Admin = createAdminSession(slave1Locator, "Slave1");

        ObjectPrx(communicator, "test")->ice_locator(masterLocator)->ice_locatorCacheTimeout(0)->ice_ping();
        ObjectPrx(communicator, "test")->ice_locator(slave1Locator)->ice_locatorCacheTimeout(0)->ice_ping();
        ObjectPrx(communicator, "test")->ice_locator(slave2Locator)->ice_locatorCacheTimeout(0)->ice_ping();

        masterAdmin->stopServer("Server");

        waitForReplicaState(masterAdmin, "Slave1", true);
        waitForReplicaState(masterAdmin, "Slave2", true);

        auto info = masterAdmin->getApplicationInfo("TestApp");
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

        Ice::LocatorPrx slave3Locator(communicator, "RepTestIceGrid/Locator-Slave3 -e 1.0:default -p 12053");
        auto slave3Admin = createAdminSession(slave3Locator, "Slave3");
        waitForNodeState(slave3Admin, "Node2", true);

        ApplicationDescriptor app;
        app.name = "TestApp";
        app.description = "added application";

        auto server = make_shared<ServerDescriptor>();
        server->id = "Server";
        server->exe = communicator->getProperties()->getProperty("ServerDir") + "/server";
        server->pwd = ".";
        server->allocatable = false;
        PropertyDescriptor prop{"Ice.Admin.Endpoints", "tcp -h 127.0.0.1"};
        server->propertySet.properties.push_back(prop);
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

        ObjectPrx(communicator, "test -e 1.0")
            ->ice_locator(masterLocator->ice_encodingVersion(Ice::Encoding_1_0))
            ->ice_locatorCacheTimeout(0)
            ->ice_ping();
        ObjectPrx(communicator, "test -e 1.0")
            ->ice_locator(slave1Locator->ice_encodingVersion(Ice::Encoding_1_0))
            ->ice_locatorCacheTimeout(0)
            ->ice_ping();
        ObjectPrx(communicator, "test -e 1.0")->ice_locator(slave3Locator)->ice_locatorCacheTimeout(0)->ice_ping();
        masterAdmin->stopServer("Server");

        masterAdmin->removeApplication("TestApp");
    }
    cout << "ok" << endl;

#if !defined(_WIN32) // This test relies on SIGSTOP/SITCONT
    cout << "testing unreachable nodes... " << flush;
    {
        ApplicationDescriptor app;
        app.name = "TestApp";
        app.description = "added application";

        app.replicaGroups.resize(1);
        app.replicaGroups[0].id = "TestReplicaGroup";
        app.replicaGroups[0].loadBalancing = make_shared<IceGrid::RandomLoadBalancingPolicy>("2");

        ObjectDescriptor object;
        object.id = Ice::stringToIdentity("test");
        object.type = "::Test::TestIntf";
        app.replicaGroups[0].objects.push_back(object);

        auto server = make_shared<ServerDescriptor>();
        server->id = "Server1";
        server->exe = communicator->getProperties()->getProperty("ServerDir") + "/server";
        server->pwd = ".";
        server->allocatable = false;
        server->propertySet.properties.push_back(PropertyDescriptor{"Ice.Admin.Endpoints", "tcp -h 127.0.0.1"});
        server->activation = "on-demand";
        AdapterDescriptor adapter;
        adapter.name = "TestAdapter";
        adapter.id = "TestAdapter.${server}";
        adapter.replicaGroupId = "TestReplicaGroup";
        adapter.serverLifetime = true;
        adapter.registerProcess = false;
        PropertyDescriptor property;
        property.name = "TestAdapter.Endpoints";
        property.value = "default";
        server->propertySet.properties.push_back(property);
        property.name = "Identity";
        property.value = "test";
        server->propertySet.properties.push_back(property);

        server->adapters.push_back(adapter);
        app.nodes["Node1"].servers.push_back(server);

        auto server2 = dynamic_pointer_cast<ServerDescriptor>(server->ice_clone());
        server2->id = "Server2";
        app.nodes["Node2"].servers.push_back(server2);

        try
        {
            masterAdmin->addApplication(app);
        }
        catch (const DeploymentException& ex)
        {
            cerr << ex.reason << endl;
        }

        ObjectPrx(communicator, "test -e 1.0@TestReplicaGroup")
            ->ice_locator(masterLocator->ice_encodingVersion(Ice::Encoding_1_0))
            ->ice_locatorCacheTimeout(0)
            ->ice_ping();
        ObjectPrx(communicator, "test -e 1.0@TestAdapter.Server1")
            ->ice_locator(masterLocator->ice_encodingVersion(Ice::Encoding_1_0))
            ->ice_locatorCacheTimeout(0)
            ->ice_ping();
        ObjectPrx(communicator, "test -e 1.0@TestAdapter.Server2")
            ->ice_locator(masterLocator->ice_encodingVersion(Ice::Encoding_1_0))
            ->ice_locatorCacheTimeout(0)
            ->ice_ping();

        auto query = QueryPrx(communicator, "RepTestIceGrid/Query")->ice_locator(masterLocator);
        test(query->findAllReplicas(ObjectPrx(communicator, "test")).size() == 2);
        test(masterAdmin->getAdapterInfo("TestReplicaGroup").size() == 2);

        admin->sendSignal("Node2", "SIGSTOP");
        try
        {
            // Wait for Node2 to be stopped by getting the TestAdapter.Server2 endpoints
            while (true)
            {
                masterAdmin->ice_invocationTimeout(100)->getAdapterInfo("TestAdapter.Server2");
            }
        }
        catch (const Ice::InvocationTimeoutException&)
        {
        }

        test(query->findAllReplicas(ObjectPrx(communicator, "test")).size() == 2);
        try
        {
            masterAdmin->ice_invocationTimeout(1000)->getAdapterInfo("TestReplicaGroup");
            admin->sendSignal("Node2", "SIGCONT");
            test(false);
        }
        catch (const Ice::InvocationTimeoutException&)
        {
            admin->sendSignal("Node2", "SIGCONT");
        }

        test(query->findAllReplicas(ObjectPrx(communicator, "test")).size() == 2);
        test(masterAdmin->ice_invocationTimeout(1000)->getAdapterInfo("TestReplicaGroup").size() == 2);

        masterAdmin->removeApplication("TestApp");
    }
    cout << "ok" << endl;
#endif

    slave1Admin->shutdownNode("Node1");
    removeServer(admin, "Node1");

    removeServer(admin, "Slave2");

    slave1Admin->shutdown();
    removeServer(admin, "Slave1");
    masterAdmin->shutdown();
    removeServer(admin, "Master");
}
