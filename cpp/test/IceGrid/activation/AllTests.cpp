// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "IceGrid/IceGrid.h"
#include "Test.h"
#include "TestHelper.h"

#include <chrono>
#include <thread>

using namespace std;
using namespace Test;

void
waitForServerState(optional<IceGrid::AdminPrx> admin, const std::string& server, IceGrid::ServerState state)
{
    int nRetry = 0;
    IceGrid::ServerState currentState = admin->getServerState(server);
    while (currentState != state && nRetry < 15)
    {
        this_thread::sleep_for(chrono::milliseconds(500));
        ++nRetry;
        currentState = admin->getServerState(server);
    }

    if (currentState != state)
    {
        cerr << "server state change timed out:" << endl;
        cerr << "server: " << server << endl;
        cerr << "expected state: " << state << endl;
        cerr << "current state: " << currentState << endl;
        test(false);
    }
}

class Pinger
{
public:
    Pinger(Ice::ObjectPrx proxy, int nRepetitions)
        : _proxy(std::move(proxy)),

          _nRepetitions(nRepetitions)
    {
    }

    virtual ~Pinger() = default;

    virtual void run()
    {
        for (int i = 0; i < _nRepetitions; ++i)
        {
            try
            {
                _proxy->ice_ping();
            }
            catch (const Ice::LocalException&)
            {
                _exception = current_exception();
            }
            catch (...)
            {
                assert(false);
            }
        }

        unique_lock lock(_mutex);
        _finished = true;
        _condVar.notify_all();
    }

    exception_ptr waitUntilFinished()
    {
        unique_lock lock(_mutex);
        while (!_finished)
        {
            _condVar.wait(lock);
        }
        return _exception;
    }

private:
    Ice::ObjectPrx _proxy;
    exception_ptr _exception;
    bool _finished{false};
    int _nRepetitions;
    mutex _mutex;
    condition_variable _condVar;
};

void
allTests(Test::TestHelper* helper)
{
    Ice::CommunicatorPtr communicator = helper->communicator();
    IceGrid::RegistryPrx registry(
        communicator,
        communicator->getDefaultLocator()->ice_getIdentity().category + "/Registry");

    IceGrid::QueryPrx query(communicator, communicator->getDefaultLocator()->ice_getIdentity().category + "/Query");

    optional<IceGrid::AdminSessionPrx> adminSession = registry->createAdminSession("foo", "bar");

    optional<IceGrid::AdminPrx> admin = adminSession->getAdmin();
    test(admin);

    admin->startServer("node-1");
    admin->startServer("node-2");

    int nRetry = 0;
    while (!admin->pingNode("node-1") && nRetry < 15)
    {
        this_thread::sleep_for(chrono::milliseconds(200));
        ++nRetry;
    }

    nRetry = 0;
    while (!admin->pingNode("node-2") && nRetry < 15)
    {
        this_thread::sleep_for(chrono::milliseconds(200));
        ++nRetry;
    }

    cout << "testing on-demand activation... " << flush;
    try
    {
        test(admin->getServerState("server") == IceGrid::ServerState::Inactive);
        TestIntfPrx obj(communicator, "server");
        obj->ice_ping();
        waitForServerState(admin, "server", IceGrid::ServerState::Active);
        obj->shutdown();
        waitForServerState(admin, "server", IceGrid::ServerState::Inactive);
        nRetry = 4;
        while (--nRetry > 0)
        {
            obj->shutdown();
        }
        waitForServerState(admin, "server", IceGrid::ServerState::Inactive);
    }
    catch (const Ice::LocalException& ex)
    {
        cerr << ex << endl;
        test(false);
    }
    cout << "ok" << endl;

    cout << "testing manual activation... " << flush;
    try
    {
        test(admin->getServerState("server-manual") == IceGrid::ServerState::Inactive);
        try
        {
            TestIntfPrx(communicator, "server-manual")->ice_ping();
            test(false);
        }
        catch (const Ice::NoEndpointException&)
        {
        }
        test(admin->getServerState("server-manual") == IceGrid::ServerState::Inactive);
        admin->startServer("server-manual");
        test(admin->getServerState("server-manual") == IceGrid::ServerState::Active);
        TestIntfPrx obj(communicator, "server-manual");
        obj->ice_ping();
        test(admin->getServerState("server-manual") == IceGrid::ServerState::Active);
        obj->shutdown();
        waitForServerState(admin, "server-manual", IceGrid::ServerState::Inactive);
    }
    catch (const Ice::LocalException& ex)
    {
        cerr << ex << endl;
        test(false);
    }
    cout << "ok" << endl;

    cout << "testing always activation... " << flush;
    try
    {
        waitForServerState(admin, "server-always", IceGrid::ServerState::Active);
        TestIntfPrx obj(communicator, "server-always");
        admin->stopServer("server-always");
        waitForServerState(admin, "server-always", IceGrid::ServerState::Active);
        obj->shutdown();
        waitForServerState(admin, "server-always", IceGrid::ServerState::Active);
        nRetry = 4;
        while (--nRetry > 0)
        {
            obj->shutdown();
        }
        waitForServerState(admin, "server-always", IceGrid::ServerState::Active);
    }
    catch (const Ice::LocalException& ex)
    {
        cerr << ex << endl;
        test(false);
    }
    cout << "ok" << endl;

    cout << "testing session activation... " << flush;
    try
    {
        optional<IceGrid::SessionPrx> session = registry->createSession("test", "");

        test(admin->getServerState("server-session") == IceGrid::ServerState::Inactive);
        TestIntfPrx obj(communicator, "server-session");
        try
        {
            obj->ice_ping();
            test(false);
        }
        catch (const Ice::NoEndpointException&)
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
        while (--nRetry > 0)
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
        catch (const Ice::NoEndpointException&)
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
        catch (const Ice::NoEndpointException&)
        {
        }
        waitForServerState(admin, "server-session", IceGrid::ServerState::Inactive);
    }
    catch (const Ice::LocalException& ex)
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
            Ice::ObjectPrx(communicator, "server")->ice_ping();
            test(false);
        }
        catch (const Ice::NoEndpointException&)
        {
        }

        while (query->findAllObjectsByType("Test").size() != count - 1)
        {
            // The notification of the server being disabled is asynchronous and might
            // not be visible to the Query interface immediately.
            this_thread::sleep_for(chrono::milliseconds(100));
        }

        try
        {
            admin->startServer("server");
            test(false);
        }
        catch (const IceGrid::ServerStartException&)
        {
        }
        test(admin->getServerState("server") == IceGrid::ServerState::Inactive);

        test(admin->getServerState("server-manual") == IceGrid::ServerState::Inactive);
        admin->enableServer("server-manual", false);
        try
        {
            Ice::ObjectPrx(communicator, "server-manual")->ice_ping();
            test(false);
        }
        catch (const Ice::NoEndpointException&)
        {
        }
        try
        {
            admin->startServer("server-manual");
            test(false);
        }
        catch (const IceGrid::ServerStartException&)
        {
        }
        test(admin->getServerState("server-manual") == IceGrid::ServerState::Inactive);
        while (query->findAllObjectsByType("Test").size() != count - 2)
        {
            // The notification of the server being disabled is asynchronous and might
            // not be visible to the Query interface immediately.
            this_thread::sleep_for(chrono::milliseconds(100));
        }

        test(admin->getServerState("server-always") == IceGrid::ServerState::Active);
        admin->enableServer("server-always", false);
        admin->stopServer("server-always");
        test(admin->getServerState("server-always") == IceGrid::ServerState::Inactive);
        try
        {
            Ice::ObjectPrx(communicator, "server-always")->ice_ping();
            test(false);
        }
        catch (const Ice::NoEndpointException&)
        {
        }
        try
        {
            admin->startServer("server-always");
            test(false);
        }
        catch (const IceGrid::ServerStartException&)
        {
        }
        test(admin->getServerState("server-always") == IceGrid::ServerState::Inactive);
        while (query->findAllObjectsByType("Test").size() != count - 3)
        {
            // The notification of the server being disabled is asynchronous and might
            // not be visible to the Query interface immediately.
            this_thread::sleep_for(chrono::milliseconds(100));
        }

        test(admin->getServerState("server") == IceGrid::ServerState::Inactive);
        admin->enableServer("server", true);
        Ice::ObjectPrx(communicator, "server")->ice_locatorCacheTimeout(0)->ice_ping();
        int pid = admin->getServerPid("server");
        admin->enableServer("server", false);
        test(admin->getServerState("server") == IceGrid::ServerState::Active);
        try
        {
            Ice::ObjectPrx(communicator, "server")->ice_locatorCacheTimeout(0)->ice_ping();
            test(false);
        }
        catch (const Ice::NoEndpointException&)
        {
        }
        admin->enableServer("server", true);
        Ice::ObjectPrx(communicator, "server")->ice_locatorCacheTimeout(0)->ice_ping();
        test(admin->getServerPid("server") == pid);
        admin->stopServer("server");
        test(admin->getServerState("server") == IceGrid::ServerState::Inactive);

        while (query->findAllObjectsByType("Test").size() != count - 2)
        {
            // The notification of the server being disabled is asynchronous and might
            // not be visible to the Query interface immediately.
            this_thread::sleep_for(chrono::milliseconds(100));
        }
    }
    catch (const Ice::LocalException& ex)
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
        Ice::ObjectPrx(communicator, "server")->ice_ping();
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
            Ice::ObjectPrx(communicator, "server-manual")->ice_ping();
            test(false);
        }
        catch (const Ice::NoEndpointException&)
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
        try
        {
            admin->startServer("server-always");
            //          test(false);
        }
        catch (const IceGrid::ServerStartException&)
        {
        }
        test(admin->getServerState("server-always") == IceGrid::ServerState::Active);
    }
    catch (const Ice::LocalException& ex)
    {
        cerr << ex << endl;
        test(false);
    }
    cout << "ok" << endl;

    cout << "testing activation failure... " << flush;
    try
    {
        int i;
        const int nThreads = 3;
        Ice::ObjectPrx invalid(communicator, "invalid-exe");
        vector<pair<shared_ptr<Pinger>, thread>> pingers;
        for (i = 0; i < nThreads; i++)
        {
            auto pinger = make_shared<Pinger>(invalid, 10);
            auto t = thread([pinger]() { pinger->run(); });
            pingers.emplace_back(pinger, std::move(t));
        }

        for (const auto& p : pingers)
        {
            exception_ptr ex = p.first->waitUntilFinished();
            try
            {
                rethrow_exception(ex);
            }
            catch (const Ice::NoEndpointException&)
            {
            }
            catch (...)
            {
                test(false);
            }
        }

        for (auto& p : pingers)
        {
            p.second.join();
        }
        pingers.clear();

        invalid = Ice::ObjectPrx(communicator, "invalid-pwd");
        for (i = 0; i < nThreads; i++)
        {
            auto pinger = make_shared<Pinger>(invalid, 10);
            auto t = thread([pinger]() { pinger->run(); });
            pingers.emplace_back(pinger, std::move(t));
        }

        for (const auto& p : pingers)
        {
            exception_ptr ex = p.first->waitUntilFinished();
            try
            {
                rethrow_exception(ex);
            }
            catch (const Ice::NoEndpointException&)
            {
            }
            catch (...)
            {
                test(false);
            }
        }

        for (auto& p : pingers)
        {
            p.second.join();
        }
        pingers.clear();

        invalid = Ice::ObjectPrx(communicator, "fail-on-startup");
        for (i = 0; i < nThreads; i++)
        {
            auto pinger = make_shared<Pinger>(invalid, 5);
            auto t = thread([pinger]() { pinger->run(); });
            pingers.emplace_back(pinger, std::move(t));
        }

        for (const auto& p : pingers)
        {
            exception_ptr ex = p.first->waitUntilFinished();
            try
            {
                rethrow_exception(ex);
            }
            catch (const Ice::NoEndpointException&)
            {
            }
            catch (...)
            {
                test(false);
            }
        }

        for (auto& p : pingers)
        {
            p.second.join();
        }
        pingers.resize(0);

        try
        {
            admin->startServer("invalid-pwd-no-oa");
            test(false);
        }
        catch (const IceGrid::ServerStartException& ex)
        {
            test(!ex.reason.empty());
        }

        try
        {
            admin->startServer("invalid-exe-no-oa");
            test(false);
        }
        catch (const IceGrid::ServerStartException& ex)
        {
            test(!ex.reason.empty());
        }
    }
    catch (const Ice::LocalException& ex)
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
        Ice::ObjectPrx proxy(communicator, "server-activation-timeout");
        vector<pair<shared_ptr<Pinger>, thread>> pingers;
        for (int i = 0; i < nThreads; i++)
        {
            auto pinger = make_shared<Pinger>(proxy, 1);
            auto t = thread([pinger]() { pinger->run(); });
            pingers.emplace_back(pinger, std::move(t));
        }

        for (const auto& p : pingers)
        {
            exception_ptr ex = p.first->waitUntilFinished();
            try
            {
                rethrow_exception(ex);
            }
            catch (const Ice::NoEndpointException&)
            {
            }
            catch (...)
            {
                test(false);
            }
        }

        for (auto& p : pingers)
        {
            p.second.join();
        }
        admin->stopServer("server-activation-timeout");
    }
    catch (const IceGrid::ServerStopException& ex)
    {
        cerr << ex << ": " << ex.reason << endl;
        test(false);
    }
    catch (const Ice::LocalException& ex)
    {
        cerr << ex << endl;
        test(false);
    }
    cout << "ok" << endl;

    cout << "testing deactivation timeout... " << flush;
    try
    {
        test(admin->getServerState("server-deactivation-timeout") == IceGrid::ServerState::Inactive);
        Ice::ObjectPrx(communicator, "server-deactivation-timeout")->ice_ping();
        admin->stopServer("server-deactivation-timeout");
    }
    catch (const Ice::LocalException& ex)
    {
        cerr << ex << endl;
        test(false);
    }
    cout << "ok" << endl;

    cout << "testing permanent disable on failure... " << flush;
    try
    {
        test(admin->getServerState("server1") == IceGrid::ServerState::Inactive);
        TestIntfPrx obj(communicator, "server1");
        obj->ice_ping();
        waitForServerState(admin, "server1", IceGrid::ServerState::Active);
        obj->fail();
        waitForServerState(admin, "server1", IceGrid::ServerState::Inactive);
        try
        {
            obj->ice_ping();
            test(false);
        }
        catch (const Ice::NoEndpointException&)
        {
        }
        test(!admin->isServerEnabled("server1"));

        test(admin->getServerState("server1-manual") == IceGrid::ServerState::Inactive);
        admin->startServer("server1-manual");
        test(admin->getServerState("server1-manual") == IceGrid::ServerState::Active);
        obj = TestIntfPrx(communicator, "server1-manual");
        obj->ice_ping();
        test(admin->getServerState("server1-manual") == IceGrid::ServerState::Active);
        obj->fail();
        waitForServerState(admin, "server1-manual", IceGrid::ServerState::Inactive);
        test(!admin->isServerEnabled("server1-manual"));

        test(admin->getServerState("server1-always") == IceGrid::ServerState::Active);
        obj = TestIntfPrx(communicator, "server1-always");
        obj->ice_ping();
        obj->fail();
        waitForServerState(admin, "server1-always", IceGrid::ServerState::Inactive);
        test(!admin->isServerEnabled("server1-always"));
    }
    catch (const Ice::LocalException& ex)
    {
        cerr << ex << endl;
        test(false);
    }
    cout << "ok" << endl;

    cout << "testing temporary disable on failure... " << flush;
    try
    {
        test(admin->getServerState("server2") == IceGrid::ServerState::Inactive);
        auto obj = TestIntfPrx(communicator, "server2");
        obj = obj->ice_locatorCacheTimeout(0);
        while (true)
        {
            obj->ice_ping();
            waitForServerState(admin, "server2", IceGrid::ServerState::Active);
            auto now = std::chrono::steady_clock::now();
            obj->fail();
            waitForServerState(admin, "server2", IceGrid::ServerState::Inactive);
            try
            {
                obj->ice_ping();
                test(chrono::steady_clock::now() - now >= 3s);
            }
            catch (const Ice::NoEndpointException&)
            {
                if (!admin->isServerEnabled("server2"))
                {
                    break; // Success
                }
            }
        }
        test(!admin->isServerEnabled("server2"));

        // Wait for the server to be re-enabled after IceGrid.Node.DisableOnFailure timeout.
        nRetry = 0;
        while (nRetry++ < 15)
        {
            try
            {
                obj->ice_ping();
                break;
            }
            catch (const Ice::NoEndpointException&)
            {
                this_thread::sleep_for(chrono::milliseconds(500));
            }
        }
        test(admin->isServerEnabled("server2"));
        waitForServerState(admin, "server2", IceGrid::ServerState::Active);
        admin->stopServer("server2");

        test(admin->getServerState("server2-manual") == IceGrid::ServerState::Inactive);
        admin->startServer("server2-manual");
        test(admin->getServerState("server2-manual") == IceGrid::ServerState::Active);
        obj = TestIntfPrx(communicator, "server2-manual");
        obj->fail();
        waitForServerState(admin, "server2-manual", IceGrid::ServerState::Inactive);
        test(!admin->isServerEnabled("server2-manual"));
        admin->startServer("server2-manual");
        test(admin->isServerEnabled("server2-manual"));
        test(admin->getServerState("server2-manual") == IceGrid::ServerState::Active);
        admin->stopServer("server2-manual");

        test(admin->getServerState("server2-always") == IceGrid::ServerState::Active);
        obj = TestIntfPrx(communicator, "server2-always");
        obj->fail();
        waitForServerState(admin, "server2-always", IceGrid::ServerState::Inactive);
        test(!admin->isServerEnabled("server2-always"));
        nRetry = 0;
        while ((!admin->isServerEnabled("server2-always") ||
                admin->getServerState("server2-always") != IceGrid::ServerState::Active) &&
               nRetry < 15)
        {
            this_thread::sleep_for(chrono::milliseconds(500));
            ++nRetry;
        }
        test(
            admin->isServerEnabled("server2-always") &&
            admin->getServerState("server2-always") == IceGrid::ServerState::Active);
        obj->ice_ping();
    }
    catch (const Ice::LocalException& ex)
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
        for (int i = 0; i < nServers; ++i)
        {
            ostringstream id;
            id << "server-" << i;
            IceGrid::ServerInstanceDescriptor server;
            server.templateName = "Server";
            server.parameterValues["id"] = id.str();
            testApp.nodes["localnode"].serverInstances.push_back(server);
        }
        try
        {
            admin->addApplication(testApp);
        }
        catch (const IceGrid::DeploymentException& ex)
        {
            cerr << ex.reason << endl;
            test(false);
        }
        try
        {
            for (int i = 0; i < nServers; ++i)
            {
                ostringstream id;
                id << "server-" << i;
                admin->startServer(id.str());
            }
        }
        catch (const IceGrid::ServerStartException& ex)
        {
            cerr << ex.reason << endl;
            test(false);
        }
        for (int i = 0; i < nServers; ++i)
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
