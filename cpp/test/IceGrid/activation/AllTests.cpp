//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <IceGrid/IceGrid.h>
#include <TestHelper.h>
#include <Test.h>

using namespace std;
using namespace Test;

void
waitForServerState(const shared_ptr<IceGrid::AdminPrx>& admin, const std::string& server, IceGrid::ServerState state)
{
    int nRetry = 0;
    while(admin->getServerState(server) != state && nRetry < 15)
    {
        this_thread::sleep_for(500ms);
        ++nRetry;
    }
    if(admin->getServerState(server) != state)
    {
        cerr << "server state change timed out:" << endl;
        cerr << "server: " << server << endl;
        cerr << "state: " << static_cast<int>(state) << endl;
    }
}

exception_ptr
ping(const shared_ptr<Ice::ObjectPrx>& proxy, int nRepetitions)
{
    exception_ptr exception = nullptr;
    for(int i = 0; i < nRepetitions; ++i)
    {
        try
        {
            proxy->ice_ping();
        }
        catch(const Ice::LocalException&)
        {
            exception = std::current_exception();
        }
        catch(...)
        {
            assert(false);
        }
    }

    return exception;
}

void
allTests(Test::TestHelper* helper)
{
    auto communicator = helper->communicator();
    auto registry = Ice::checkedCast<IceGrid::RegistryPrx>(
        communicator->stringToProxy(communicator->getDefaultLocator()->ice_getIdentity().category + "/Registry"));
    test(registry);

    auto query = Ice::checkedCast<IceGrid::QueryPrx>(
        communicator->stringToProxy(communicator->getDefaultLocator()->ice_getIdentity().category + "/Query"));

    auto adminSession = registry->createAdminSession("foo", "bar");

    adminSession->ice_getConnection()->setACM(registry->getACMTimeout(),
                                              Ice::nullopt,
                                              Ice::ACMHeartbeat::HeartbeatAlways);

    auto admin = adminSession->getAdmin();
    test(admin);

    admin->startServer("node-1");
    admin->startServer("node-2");

    int nRetry = 0;
    while(!admin->pingNode("node-1") && nRetry < 15)
    {
        this_thread::sleep_for(200ms);
        ++nRetry;
    }

    nRetry = 0;
    while(!admin->pingNode("node-2") && nRetry < 15)
    {
        this_thread::sleep_for(200ms);
        ++nRetry;
    }

    cout << "testing on-demand activation... " << flush;
    try
    {
        test(admin->getServerState("server") == IceGrid::ServerState::Inactive);
        auto obj = Ice::checkedCast<TestIntfPrx>(communicator->stringToProxy("server"));
        waitForServerState(admin, "server", IceGrid::ServerState::Active);
        obj->shutdown();
        waitForServerState(admin, "server", IceGrid::ServerState::Inactive);
        nRetry = 4;
        while(--nRetry > 0)
        {
            obj->shutdown();
        }
        waitForServerState(admin, "server", IceGrid::ServerState::Inactive);
    }
    catch(const Ice::LocalException& ex)
    {
        cerr << ex << endl;
        test(false);
    }
    cout << "ok" << endl;

    cout << "testing manual activation... " << flush;
    try
    {
        test(admin->getServerState("server-manual") == IceGrid::ServerState::Inactive);
        shared_ptr<TestIntfPrx> obj;
        try
        {
            obj = Ice::checkedCast<TestIntfPrx>(communicator->stringToProxy("server-manual"));
            test(false);
        }
        catch(const Ice::NoEndpointException&)
        {
        }
        test(admin->getServerState("server-manual") == IceGrid::ServerState::Inactive);
        admin->startServer("server-manual");
        test(admin->getServerState("server-manual") == IceGrid::ServerState::Active);
        obj = Ice::checkedCast<TestIntfPrx>(communicator->stringToProxy("server-manual"));
        test(admin->getServerState("server-manual") == IceGrid::ServerState::Active);
        obj->shutdown();
        waitForServerState(admin, "server-manual", IceGrid::ServerState::Inactive);
    }
    catch(const Ice::LocalException& ex)
    {
        cerr << ex << endl;
        test(false);
    }
    cout << "ok" << endl;

    cout << "testing always activation... " << flush;
    try
    {
        waitForServerState(admin, "server-always", IceGrid::ServerState::Active);
        auto obj = Ice::checkedCast<TestIntfPrx>(communicator->stringToProxy("server-always"));
        admin->stopServer("server-always");
        waitForServerState(admin, "server-always", IceGrid::ServerState::Active);
        obj->shutdown();
        waitForServerState(admin, "server-always", IceGrid::ServerState::Active);
        nRetry = 4;
        while(--nRetry > 0)
        {
            obj->shutdown();
        }
        waitForServerState(admin, "server-always", IceGrid::ServerState::Active);
    }
    catch(const Ice::LocalException& ex)
    {
        cerr << ex << endl;
        test(false);
    }
    cout << "ok" << endl;

    cout << "testing session activation... " << flush;
    try
    {
        auto session = registry->createSession("test", "");

        test(admin->getServerState("server-session") == IceGrid::ServerState::Inactive);
        auto obj = Ice::uncheckedCast<TestIntfPrx>(communicator->stringToProxy("server-session"));
        try
        {
            obj->ice_ping();
            test(false);
        }
        catch(const Ice::NoEndpointException&)
        {
        }
        session->allocateObjectById(obj->ice_getIdentity());
        obj->ice_ping();
        waitForServerState(admin, "server-session", IceGrid::ServerState::Active);
        obj->shutdown();
        waitForServerState(admin, "server-session", IceGrid::ServerState::Inactive);
        obj->ice_ping();
        waitForServerState(admin, "server-session", IceGrid::ServerState::Active);
        nRetry = 4;
        while(--nRetry > 0)
        {
            obj->shutdown();
        }
        obj->ice_ping();
        waitForServerState(admin, "server-session", IceGrid::ServerState::Active);
        session->releaseObject(obj->ice_getIdentity());
        try
        {
            obj->ice_ping();
            test(false);
        }
        catch(const Ice::NoEndpointException&)
        {
        }
        waitForServerState(admin, "server-session", IceGrid::ServerState::Inactive);

        session->allocateObjectById(obj->ice_getIdentity());
        obj->ice_ping();
        waitForServerState(admin, "server-session", IceGrid::ServerState::Active);
        session->destroy();
        try
        {
            obj->ice_ping();
            test(false);
        }
        catch(const Ice::NoEndpointException&)
        {
        }
        waitForServerState(admin, "server-session", IceGrid::ServerState::Inactive);
    }
    catch(const Ice::LocalException& ex)
    {
        cerr << ex << endl;
        test(false);
    }
    cout << "ok" << endl;

    cout << "testing server disable... " << flush;
    try
    {
        size_t count = query->findAllObjectsByType("Test").size();

        test(admin->getServerState("server") == IceGrid::ServerState::Inactive);
        admin->enableServer("server", false);
        try
        {
            communicator->stringToProxy("server")->ice_ping();
            test(false);
        }
        catch(const Ice::NoEndpointException&)
        {
        }

        while(query->findAllObjectsByType("Test").size() != count - 1)
        {
            // The notification of the server being disabled is asynchronous and might
            // not be visible to the Query interface immediately.
            this_thread::sleep_for(100ms);
        }

        try
        {
            admin->startServer("server");
            test(false);
        }
        catch(const IceGrid::ServerStartException&)
        {
        }
        test(admin->getServerState("server") == IceGrid::ServerState::Inactive);

        test(admin->getServerState("server-manual") == IceGrid::ServerState::Inactive);
        admin->enableServer("server-manual", false);
        try
        {
            communicator->stringToProxy("server-manual")->ice_ping();
            test(false);
        }
        catch(const Ice::NoEndpointException&)
        {
        }
        try
        {
            admin->startServer("server-manual");
            test(false);
        }
        catch(const IceGrid::ServerStartException&)
        {
        }
        test(admin->getServerState("server-manual") == IceGrid::ServerState::Inactive);
        while(query->findAllObjectsByType("Test").size() != count - 2)
        {
            // The notification of the server being disabled is asynchronous and might
            // not be visible to the Query interface immediately.
            this_thread::sleep_for(100ms);
        }

        test(admin->getServerState("server-always") == IceGrid::ServerState::Active);
        admin->enableServer("server-always", false);
        admin->stopServer("server-always");
        test(admin->getServerState("server-always") == IceGrid::ServerState::Inactive);
        try
        {
            communicator->stringToProxy("server-always")->ice_ping();
            test(false);
        }
        catch(const Ice::NoEndpointException&)
        {
        }
        try
        {
            admin->startServer("server-always");
            test(false);
        }
        catch(const IceGrid::ServerStartException&)
        {
        }
        test(admin->getServerState("server-always") == IceGrid::ServerState::Inactive);
        while(query->findAllObjectsByType("Test").size() != count - 3)
        {
            // The notification of the server being disabled is asynchronous and might
            // not be visible to the Query interface immediately.
            this_thread::sleep_for(100ms);
        }

        test(admin->getServerState("server") == IceGrid::ServerState::Inactive);
        admin->enableServer("server", true);
        communicator->stringToProxy("server")->ice_locatorCacheTimeout(0)->ice_ping();
        int pid = admin->getServerPid("server");
        admin->enableServer("server", false);
        test(admin->getServerState("server") == IceGrid::ServerState::Active);
        try
        {
            communicator->stringToProxy("server")->ice_locatorCacheTimeout(0)->ice_ping();
            test(false);
        }
        catch(const Ice::NoEndpointException&)
        {
        }
        admin->enableServer("server", true);
        communicator->stringToProxy("server")->ice_locatorCacheTimeout(0)->ice_ping();
        test(admin->getServerPid("server") == pid);
        admin->stopServer("server");
        test(admin->getServerState("server") == IceGrid::ServerState::Inactive);

        while(query->findAllObjectsByType("Test").size() != count - 2)
        {
            // The notification of the server being disabled is asynchronous and might
            // not be visible to the Query interface immediately.
            this_thread::sleep_for(100ms);
        }
    }
    catch(const Ice::LocalException& ex)
    {
        cerr << ex << endl;
        test(false);
    }
    cout << "ok" << endl;

    cout << "testing server enable... " << flush;
    try
    {
        test(admin->getServerState("server") == IceGrid::ServerState::Inactive);
        admin->enableServer("server", true);
        communicator->stringToProxy("server")->ice_ping();
        admin->stopServer("server");
        test(admin->getServerState("server") == IceGrid::ServerState::Inactive);
        admin->startServer("server");
        test(admin->getServerState("server") == IceGrid::ServerState::Active);
        admin->stopServer("server");
        test(admin->getServerState("server") == IceGrid::ServerState::Inactive);

        test(admin->getServerState("server-manual") == IceGrid::ServerState::Inactive);
        admin->enableServer("server-manual", true);
        try
        {
            communicator->stringToProxy("server-manual")->ice_ping();
            test(false);
        }
        catch(const Ice::NoEndpointException&)
        {
        }
        test(admin->getServerState("server-manual") == IceGrid::ServerState::Inactive);
        admin->startServer("server-manual");
        test(admin->getServerState("server-manual") == IceGrid::ServerState::Active);
        admin->stopServer("server-manual");
        test(admin->getServerState("server-manual") == IceGrid::ServerState::Inactive);

        test(admin->getServerState("server-always") == IceGrid::ServerState::Inactive);
        admin->enableServer("server-always", true);
        waitForServerState(admin, "server-always", IceGrid::ServerState::Active);
        admin->stopServer("server-always");

        admin->startServer("server-always");
        test(admin->getServerState("server-always") == IceGrid::ServerState::Active);
    }
    catch(const Ice::LocalException& ex)
    {
        cerr << ex << endl;
        test(false);
    }
    cout << "ok" << endl;

    cout << "testing activation failure... " << flush;
    try
    {
        auto invalid = communicator->stringToProxy("invalid-exe");

        future<exception_ptr> futures[3] = {};
        const int nThreads = 3;

        for(int i = 0; i < nThreads; i++)
        {
            futures[i] = async(launch::async, [&invalid] { return ping(invalid, 10); });
        }
        for(int i = 0; i < nThreads; i++)
        {
            try
            {
                auto exception = futures[i].get();
                test(exception);
                rethrow_exception(exception);
            }
            catch(const Ice::NoEndpointException&)
            {
            }
            catch(...)
            {
                test(false);
            }
        }

        invalid = communicator->stringToProxy("invalid-pwd");
        for(int i = 0; i < nThreads; i++)
        {
            futures[i] = async(launch::async, [&invalid] { return ping(invalid, 10); });
        }
        for(int i = 0; i < nThreads; i++)
        {
            try
            {
                auto exception = futures[i].get();
                test(exception);
                rethrow_exception(exception);
            }
            catch(const Ice::NoEndpointException&)
            {
            }
            catch(...)
            {
                test(false);
            }
        }

        invalid = communicator->stringToProxy("fail-on-startup");
        for(int i = 0; i < nThreads; i++)
        {
            futures[i] = async(launch::async, [&invalid] { return ping(invalid, 5); });
        }
        for(int i = 0; i < nThreads; i++)
        {

            try
            {
                auto exception = futures[i].get();
                test(exception);
                rethrow_exception(exception);
            }
            catch(const Ice::NoEndpointException&)
            {
            }
            catch(...)
            {
                test(false);
            }
        }

        try
        {
            admin->startServer("invalid-pwd-no-oa");
            test(false);
        }
        catch(const IceGrid::ServerStartException& ex)
        {
            test(!ex.reason.empty());
        }

        try
        {
            admin->startServer("invalid-exe-no-oa");
            test(false);
        }
        catch(const IceGrid::ServerStartException& ex)
        {
            test(!ex.reason.empty());
        }
    }
    catch(const Ice::LocalException& ex)
    {
        cerr << ex << endl;
        test(false);
    }
    cout << "ok" << endl;

    cout << "testing activation timeout... " << flush;
    try
    {
        test(admin->getServerState("server-activation-timeout") == IceGrid::ServerState::Inactive);
        const int nThreads = 5;
        auto proxy = communicator->stringToProxy("server-activation-timeout");
        vector<future<exception_ptr>> threads;
        threads.reserve(nThreads);
        for(int i = 0; i < nThreads; i++)
        {
            threads.push_back(async(launch::async, [&proxy] { return ping(proxy, 1); }));
        }
        for(auto& p: threads)
        {
            try
            {
                auto exception = p.get();
                test(exception);
                rethrow_exception(exception);
            }
            catch(const Ice::NoEndpointException&)
            {
            }
            catch(...)
            {
                test(false);
            }
        }
        admin->stopServer("server-activation-timeout");
    }
    catch(const IceGrid::ServerStopException& ex)
    {
        cerr << ex << ": " << ex.reason << endl;
        test(false);
    }
    catch(const Ice::LocalException& ex)
    {
        cerr << ex << endl;
        test(false);
    }
    cout << "ok" << endl;

    cout << "testing deactivation timeout... " << flush;
    try
    {
        test(admin->getServerState("server-deactivation-timeout") == IceGrid::ServerState::Inactive);
        communicator->stringToProxy("server-deactivation-timeout")->ice_ping();
        admin->stopServer("server-deactivation-timeout");
    }
    catch(const Ice::LocalException& ex)
    {
        cerr << ex << endl;
        test(false);
    }
    cout << "ok" << endl;

    cout << "testing permanent disable on failure... " << flush;
    try
    {
        test(admin->getServerState("server1") == IceGrid::ServerState::Inactive);
        auto obj = Ice::checkedCast<TestIntfPrx>(communicator->stringToProxy("server1"));
        waitForServerState(admin, "server1", IceGrid::ServerState::Active);
        obj->fail();
        waitForServerState(admin, "server1", IceGrid::ServerState::Inactive);
        try
        {
            obj->ice_ping();
            test(false);
        }
        catch(const Ice::NoEndpointException&)
        {
        }
        test(!admin->isServerEnabled("server1"));

        test(admin->getServerState("server1-manual") == IceGrid::ServerState::Inactive);
        admin->startServer("server1-manual");
        test(admin->getServerState("server1-manual") == IceGrid::ServerState::Active);
        obj = Ice::checkedCast<TestIntfPrx>(communicator->stringToProxy("server1-manual"));
        test(admin->getServerState("server1-manual") == IceGrid::ServerState::Active);
        obj->fail();
        waitForServerState(admin, "server1-manual", IceGrid::ServerState::Inactive);
        test(!admin->isServerEnabled("server1-manual"));

        test(admin->getServerState("server1-always") == IceGrid::ServerState::Active);
        obj = Ice::checkedCast<TestIntfPrx>(communicator->stringToProxy("server1-always"));
        obj->fail();
        waitForServerState(admin, "server1-always", IceGrid::ServerState::Inactive);
        test(!admin->isServerEnabled("server1-always"));
    }
    catch(const Ice::LocalException& ex)
    {
        cerr << ex << endl;
        test(false);
    }
    cout << "ok" << endl;

    cout << "testing temporary disable on failure... " << flush;
    try
    {
        test(admin->getServerState("server2") == IceGrid::ServerState::Inactive);
        auto obj = Ice::uncheckedCast<TestIntfPrx>(communicator->stringToProxy("server2"));
        while(true)
        {
            obj->ice_ping();
            waitForServerState(admin, "server2", IceGrid::ServerState::Active);
            auto now = std::chrono::system_clock::now();
            obj->fail();
            waitForServerState(admin, "server2", IceGrid::ServerState::Inactive);
            try
            {
                obj->ice_ping();
                test(chrono::system_clock::now() - now >= 2s);
            }
            catch(const Ice::NoEndpointException&)
            {
                break; // Success
            }
        }
        test(!admin->isServerEnabled("server2"));
        nRetry = 0;
        while(!admin->isServerEnabled("server2") && nRetry < 15)
        {
            this_thread::sleep_for(500ms);
            ++nRetry;
            try
            {
                obj->ice_ping();
            }
            catch(const Ice::NoEndpointException&)
            {
            }
        }
        test(admin->isServerEnabled("server2"));
        waitForServerState(admin, "server2", IceGrid::ServerState::Active);
        obj->ice_ping();
        admin->stopServer("server2");

        test(admin->getServerState("server2-manual") == IceGrid::ServerState::Inactive);
        admin->startServer("server2-manual");
        test(admin->getServerState("server2-manual") == IceGrid::ServerState::Active);
        obj = Ice::checkedCast<TestIntfPrx>(communicator->stringToProxy("server2-manual"));
        obj->fail();
        waitForServerState(admin, "server2-manual", IceGrid::ServerState::Inactive);
        test(!admin->isServerEnabled("server2-manual"));
        admin->startServer("server2-manual");
        test(admin->isServerEnabled("server2-manual"));
        test(admin->getServerState("server2-manual") == IceGrid::ServerState::Active);
        admin->stopServer("server2-manual");

        test(admin->getServerState("server2-always") == IceGrid::ServerState::Active);
        obj = Ice::checkedCast<TestIntfPrx>(communicator->stringToProxy("server2-always"));
        obj->fail();
        waitForServerState(admin, "server2-always", IceGrid::ServerState::Inactive);
        test(!admin->isServerEnabled("server2-always"));
        nRetry = 0;
        while((!admin->isServerEnabled("server2-always") ||
               admin->getServerState("server2-always") != IceGrid::ServerState::Active) &&
              nRetry < 15)
        {
            this_thread::sleep_for(500ms);
            ++nRetry;
        }
        test(admin->isServerEnabled("server2-always") && admin->getServerState("server2-always") == IceGrid::ServerState::Active);
        obj->ice_ping();
    }
    catch(const Ice::LocalException& ex)
    {
        cerr << ex << endl;
        test(false);
    }
    cout << "ok" << endl;

    cout << "testing large number of servers... " << flush;
    {
        IceGrid::ApplicationInfo info = admin->getApplicationInfo("Test");
        IceGrid::ApplicationDescriptor testApp;
        testApp.name = "TestApp";
        testApp.serverTemplates = info.descriptor.serverTemplates;
        testApp.variables = info.descriptor.variables;
        const int nServers = 75;
        for(int i = 0; i < nServers; ++i)
        {
            ostringstream id;
            id << "server-" << i;
            IceGrid::ServerInstanceDescriptor server;
            server._cpp_template = "Server";
            server.parameterValues["id"] = id.str();
            testApp.nodes["localnode"].serverInstances.push_back(server);
        }
        try
        {
            admin->addApplication(testApp);
        }
        catch(const IceGrid::DeploymentException& ex)
        {
            cerr << ex.reason << endl;
            test(false);
        }
        try
        {
            for(int i = 0; i < nServers; ++i)
            {
                ostringstream id;
                id << "server-" << i;
                admin->startServer(id.str());
            }
        }
        catch(const IceGrid::ServerStartException& ex)
        {
            cerr << ex.reason << endl;
            test(false);
        }
        for(int i = 0; i < nServers; ++i)
        {
            ostringstream id;
            id << "server-" << i;
            admin->stopServer(id.str());
        }
        admin->removeApplication("TestApp");
    }
    cout << "ok" << endl;

    admin->stopServer("node-1");
    admin->stopServer("node-2");

    adminSession->destroy();
}
