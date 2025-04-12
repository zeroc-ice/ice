// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "Test.h"
#include "TestHelper.h"

#include <stdexcept>
#include <thread>

using namespace std;
using namespace Ice;
using namespace Test;

namespace
{
    EndpointInfoPtr getUnderlying(const EndpointInfoPtr& info)
    {
        return info->underlying ? getUnderlying(info->underlying) : info;
    }
}

void
allTests(Test::TestHelper* helper)
{
    CommunicatorPtr communicator = helper->communicator();
    TestIntfPrx obj(communicator, "test:" + helper->getTestEndpoint());

    {
        if (communicator->getProperties()->getIceProperty("Ice.Default.Protocol") != "ssl" &&
            communicator->getProperties()->getIceProperty("Ice.Default.Protocol") != "wss")
        {
            cout << "creating/destroying/recreating object adapter... " << flush;
            ObjectAdapterPtr adpt = communicator->createObjectAdapterWithEndpoints("TransientTestAdapter", "default");
            try
            {
                communicator->createObjectAdapterWithEndpoints("TransientTestAdapter", "default");
                test(false);
            }
            catch (const AlreadyRegisteredException&)
            {
            }
            adpt->destroy();

            adpt = communicator->createObjectAdapterWithEndpoints("TransientTestAdapter", "default");
            adpt->destroy();
            cout << "ok" << endl;
        }
    }

    cout << "creating/activating/deactivating object adapter in one operation... " << flush;
    obj->transient();
    obj->transientAsync().get();
    cout << "ok" << endl;

    {
        cout << "testing connection closure... " << flush;
        for (int i = 0; i < 10; ++i)
        {
            Ice::InitializationData initData;
            initData.properties = communicator->getProperties()->clone();
            installTransport(initData);
            Ice::CommunicatorHolder comm(initData);
            comm->stringToProxy("test:" + helper->getTestEndpoint())->ice_pingAsync(nullptr);
        }
        cout << "ok" << endl;
    }

    cout << "testing object adapter published endpoints... " << flush;
    {
        communicator->getProperties()->setProperty("PAdapter.PublishedEndpoints", "tcp -h localhost -p 12345 -t 30000");
        Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("PAdapter");
        test(adapter->getPublishedEndpoints().size() == 1);
        Ice::EndpointPtr endpt = adapter->getPublishedEndpoints()[0];
        test(endpt->toString() == "tcp -h localhost -p 12345 -t 30000");
        Ice::ObjectPrx prx(communicator, "dummy:tcp -h localhost -p 12346 -t 20000:tcp -h localhost -p 12347 -t 10000");
        adapter->setPublishedEndpoints(prx->ice_getEndpoints());
        test(adapter->getPublishedEndpoints().size() == 2);
        Ice::Identity id;
        id.name = "dummy";
        test(adapter->createProxy(id)->ice_getEndpoints() == prx->ice_getEndpoints());
        test(adapter->getPublishedEndpoints() == prx->ice_getEndpoints());
        adapter->destroy();
        test(adapter->getPublishedEndpoints().empty());
    }
    cout << "ok" << endl;

    cout << "testing object adapter published host... " << flush;
    {
        communicator->getProperties()->setProperty("PHAdapter.Endpoints", "default -h *");

        // PublishedHost not set
        {
            ObjectAdapterPtr adapter = communicator->createObjectAdapter("PHAdapter");
            auto publishedEndpoints = adapter->getPublishedEndpoints();
            test(publishedEndpoints.size() == 1);
            auto ipEndpointInfo = dynamic_pointer_cast<IPEndpointInfo>(getUnderlying(publishedEndpoints[0]->getInfo()));
            test(ipEndpointInfo && !ipEndpointInfo->host.empty());
            adapter->destroy();
        }

        communicator->getProperties()->setProperty("PHAdapter.PublishedHost", "test.zeroc.com");
        {
            ObjectAdapterPtr adapter = communicator->createObjectAdapter("PHAdapter");
            auto publishedEndpoints = adapter->getPublishedEndpoints();
            test(publishedEndpoints.size() == 1);
            auto ipEndpointInfo = dynamic_pointer_cast<IPEndpointInfo>(getUnderlying(publishedEndpoints[0]->getInfo()));
            test(ipEndpointInfo && ipEndpointInfo->host == "test.zeroc.com");
            adapter->destroy();
        }

        // Listening on loopback
        communicator->getProperties()->setProperty("PHAdapter.Endpoints", "default -h 127.0.0.1");

        communicator->getProperties()->setProperty("PHAdapter.PublishedHost", "");
        {
            ObjectAdapterPtr adapter = communicator->createObjectAdapter("PHAdapter");
            auto publishedEndpoints = adapter->getPublishedEndpoints();
            test(publishedEndpoints.size() == 1);
            auto ipEndpointInfo = dynamic_pointer_cast<IPEndpointInfo>(getUnderlying(publishedEndpoints[0]->getInfo()));
            test(ipEndpointInfo && ipEndpointInfo->host == "127.0.0.1");
            adapter->destroy();
        }

        communicator->getProperties()->setProperty("PHAdapter.PublishedHost", "test.zeroc.com");
        {
            ObjectAdapterPtr adapter = communicator->createObjectAdapter("PHAdapter");
            auto publishedEndpoints = adapter->getPublishedEndpoints();
            test(publishedEndpoints.size() == 1);
            auto ipEndpointInfo = dynamic_pointer_cast<IPEndpointInfo>(getUnderlying(publishedEndpoints[0]->getInfo()));
            test(ipEndpointInfo && ipEndpointInfo->host == "test.zeroc.com");
            adapter->destroy();
        }

        // Two loopback endpoints with different ports
        communicator->getProperties()->setProperty(
            "PHAdapter.Endpoints",
            "default -h 127.0.0.1 -p 12345:default -h 127.0.0.1");

        communicator->getProperties()->setProperty("PHAdapter.PublishedHost", "");
        {
            ObjectAdapterPtr adapter = communicator->createObjectAdapter("PHAdapter");
            auto publishedEndpoints = adapter->getPublishedEndpoints();
            test(publishedEndpoints.size() == 2);
            auto ipEndpointInfo0 =
                dynamic_pointer_cast<IPEndpointInfo>(getUnderlying(publishedEndpoints[0]->getInfo()));
            auto ipEndpointInfo1 =
                dynamic_pointer_cast<IPEndpointInfo>(getUnderlying(publishedEndpoints[1]->getInfo()));
            test(ipEndpointInfo0 && ipEndpointInfo0->host == "127.0.0.1" && ipEndpointInfo0->port == 12345);
            test(ipEndpointInfo1 && ipEndpointInfo1->host == "127.0.0.1" && ipEndpointInfo1->port != 12345);
            adapter->destroy();
        }

        // Two endpoints - one loopback, one not loopback
        communicator->getProperties()->setProperty("PHAdapter.Endpoints", "default -h 127.0.0.1 -p 12345:default -h *");

        communicator->getProperties()->setProperty("PHAdapter.PublishedHost", "");
        {
            ObjectAdapterPtr adapter = communicator->createObjectAdapter("PHAdapter");
            auto publishedEndpoints = adapter->getPublishedEndpoints();
            test(publishedEndpoints.size() == 1); // loopback filtered out
            auto ipEndpointInfo = dynamic_pointer_cast<IPEndpointInfo>(getUnderlying(publishedEndpoints[0]->getInfo()));
            test(ipEndpointInfo && !ipEndpointInfo->host.empty() && ipEndpointInfo->port != 12345);
            adapter->destroy();
        }

        communicator->getProperties()->setProperty("PHAdapter.PublishedHost", "test.zeroc.com");
        {
            ObjectAdapterPtr adapter = communicator->createObjectAdapter("PHAdapter");
            auto publishedEndpoints = adapter->getPublishedEndpoints();
            test(publishedEndpoints.size() == 1); // loopback filtered out
            auto ipEndpointInfo = dynamic_pointer_cast<IPEndpointInfo>(getUnderlying(publishedEndpoints[0]->getInfo()));
            test(ipEndpointInfo && ipEndpointInfo->host == "test.zeroc.com" && ipEndpointInfo->port != 12345);
            adapter->destroy();
        }

        // Two non-loopback endpoints
        communicator->getProperties()->setProperty("PHAdapter.Endpoints", "tcp -h * -p 12345:default -h *");

        communicator->getProperties()->setProperty("PHAdapter.PublishedHost", "");
        {
            ObjectAdapterPtr adapter = communicator->createObjectAdapter("PHAdapter");
            auto publishedEndpoints = adapter->getPublishedEndpoints();
            test(publishedEndpoints.size() == 2);
            auto ipEndpointInfo0 =
                dynamic_pointer_cast<IPEndpointInfo>(getUnderlying(publishedEndpoints[0]->getInfo()));
            auto ipEndpointInfo1 =
                dynamic_pointer_cast<IPEndpointInfo>(getUnderlying(publishedEndpoints[1]->getInfo()));
            test(ipEndpointInfo0 && !ipEndpointInfo0->host.empty() && ipEndpointInfo0->port == 12345);
            test(ipEndpointInfo1 && !ipEndpointInfo1->host.empty() && ipEndpointInfo1->port != 12345);
            adapter->destroy();
        }

        communicator->getProperties()->setProperty("PHAdapter.PublishedHost", "test.zeroc.com");
        {
            ObjectAdapterPtr adapter = communicator->createObjectAdapter("PHAdapter");
            auto publishedEndpoints = adapter->getPublishedEndpoints();
            test(publishedEndpoints.size() == 2);
            auto ipEndpointInfo0 =
                dynamic_pointer_cast<IPEndpointInfo>(getUnderlying(publishedEndpoints[0]->getInfo()));
            auto ipEndpointInfo1 =
                dynamic_pointer_cast<IPEndpointInfo>(getUnderlying(publishedEndpoints[1]->getInfo()));
            test(ipEndpointInfo0 && ipEndpointInfo0->host == "test.zeroc.com" && ipEndpointInfo0->port == 12345);
            test(ipEndpointInfo1 && ipEndpointInfo1->host == "test.zeroc.com" && ipEndpointInfo1->port != 12345);
            adapter->destroy();
        }
    }
    cout << "ok" << endl;

    if (obj->ice_getConnection())
    {
        cout << "testing object adapter with bi-dir connection... " << flush;

        test(communicator->getDefaultObjectAdapter() == nullptr);
        test(obj->ice_getCachedConnection()->getAdapter() == nullptr);

        ObjectAdapterPtr adapter = communicator->createObjectAdapter("");

        communicator->setDefaultObjectAdapter(adapter);
        test(communicator->getDefaultObjectAdapter() == adapter);

        // create new connection
        obj->ice_getCachedConnection()->close().get();
        obj->ice_ping();

        test(obj->ice_getCachedConnection()->getAdapter() == adapter);

        // Ensure destroying the OA doesn't affect the ability to send outgoing requests.
        adapter->destroy();
        obj->ice_getCachedConnection()->close().get();
        obj->ice_ping();

        communicator->setDefaultObjectAdapter(nullptr);

        // create new connection
        obj->ice_getCachedConnection()->close().get();
        obj->ice_ping();

        adapter = communicator->createObjectAdapter("");
        test(obj->ice_getCachedConnection()->getAdapter() == nullptr);
        obj->ice_getCachedConnection()->setAdapter(adapter);
        test(obj->ice_getCachedConnection()->getAdapter() == adapter);
        obj->ice_getCachedConnection()->setAdapter(nullptr);

        adapter->destroy();
        try
        {
            obj->ice_getCachedConnection()->setAdapter(adapter);
            test(false);
        }
        catch (const Ice::ObjectAdapterDestroyedException&)
        {
        }
        cout << "ok" << endl;
    }

    cout << "testing object adapter with router... " << flush;
    {
        Ice::Identity routerId;
        routerId.name = "router";
        auto router = obj->ice_identity<Ice::RouterPrx>(routerId)->ice_connectionId("rc");
        Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapterWithRouter("", router);
        test(adapter->getPublishedEndpoints().size() == 1);
        test(adapter->getPublishedEndpoints()[0]->toString() == "tcp -h localhost -p 23456 -t 30000");
        try
        {
            adapter->setPublishedEndpoints(router->ice_getEndpoints());
            test(false);
        }
        catch (const invalid_argument&)
        {
            // Expected.
        }
        adapter->destroy();

        try
        {
            routerId.name = "test";
            router = obj->ice_identity<Ice::RouterPrx>(routerId);
            communicator->createObjectAdapterWithRouter("", router);
            test(false);
        }
        catch (const Ice::OperationNotExistException&)
        {
            // Expected: the "test" object doesn't implement Ice::Router!
        }

        try
        {
            router = Ice::RouterPrx(communicator, "test:" + helper->getTestEndpoint(1));
            communicator->createObjectAdapterWithRouter("", router);
            test(false);
        }
        catch (const Ice::ConnectFailedException&)
        {
        }
        catch (const Ice::ConnectTimeoutException&)
        {
        }

        try
        {
            router = Ice::RouterPrx(communicator, "test:" + helper->getTestEndpoint(1));
            communicator->getProperties()->setProperty("AdapterWithRouter.Endpoints", "tcp -h 127.0.0.1");
            communicator->createObjectAdapterWithRouter("AdapterWithRouter", router);
            test(false);
        }
        catch (const Ice::InitializationException&)
        {
        }
    }
    cout << "ok" << endl;

    cout << "testing object adapter creation with port in use... " << flush;
    {
        Ice::ObjectAdapterPtr adapter1 =
            communicator->createObjectAdapterWithEndpoints("Adpt1", helper->getTestEndpoint(10));
        try
        {
            communicator->createObjectAdapterWithEndpoints("Adpt2", helper->getTestEndpoint(10));
            test(false);
        }
        catch (const Ice::LocalException&)
        {
            // Expected can't re-use the same endpoint.
        }
        adapter1->destroy();
    }
    cout << "ok" << endl;

    cout << "deactivating object adapter in the server... " << flush;
    obj->deactivate();
    cout << "ok" << endl;

    cout << "testing whether server is gone... " << flush;
    if (obj->ice_getConnection()) // not collocated
    {
        try
        {
#ifdef _WIN32
            obj = obj->ice_invocationTimeout(100); // Workaround to speed up testing
#endif
            obj->ice_ping();
            test(false);
        }
        catch (const LocalException&)
        {
            cout << "ok" << endl;
        }
    }
    else
    {
        obj->ice_ping();
        cout << "ok" << endl;
    }

    cout << "testing server idle time..." << flush;
    std::thread thread1(
        []()
        {
            InitializationData idleInitData;
            idleInitData.properties = make_shared<Ice::Properties>();
            idleInitData.properties->setProperty("Ice.ServerIdleTime", "1");
#ifdef _WIN32
            // With our Windows implementation, the thread pool threads have to be idle first before server idle time is
            // checked
            idleInitData.properties->setProperty("Ice.ThreadPool.Server.ThreadIdleTime", "1");
#endif
            CommunicatorHolder idleCommunicator(idleInitData);
            // The server thread pool is started lazily so we need to create an object adapter and activate it.
            ObjectAdapterPtr idleOA = idleCommunicator->createObjectAdapterWithEndpoints("IdleOA", "tcp -h 127.0.0.1");
            idleOA->activate();
            idleCommunicator->waitForShutdown();
            idleCommunicator->destroy();
        });
    std::thread thread2(
        []()
        {
            InitializationData idleInitData;
            idleInitData.properties = make_shared<Ice::Properties>();
            idleInitData.properties->setProperty("Ice.ServerIdleTime", "0");
#ifdef _WIN32
            // With our Windows implementation, the thread pool threads have to be idle first before server idle time is
            // checked
            idleInitData.properties->setProperty("Ice.ThreadPool.Server.ThreadIdleTime", "1");
#endif
            CommunicatorHolder idleCommunicator(idleInitData);
            // The server thread pool is started lazily so we need to create an object adapter and activate it.
            ObjectAdapterPtr idleOA = idleCommunicator->createObjectAdapterWithEndpoints("IdleOA", "tcp -h 127.0.0.1");
            idleOA->activate();
            std::this_thread::sleep_for(1200ms);
            test(!idleCommunicator->isShutdown());
            idleCommunicator->destroy();
        });
    thread1.join();
    thread2.join();
    cout << "ok" << endl;
}
