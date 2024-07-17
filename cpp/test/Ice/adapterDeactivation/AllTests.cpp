//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "Ice/Ice.h"
#include "Test.h"
#include "TestHelper.h"

#include <stdexcept>

using namespace std;
using namespace Ice;
using namespace Test;

void
allTests(Test::TestHelper* helper)
{
    CommunicatorPtr communicator = helper->communicator();
    TestIntfPrx obj(communicator, "test:" + helper->getTestEndpoint());

    {
        if (communicator->getProperties()->getProperty("Ice.Default.Protocol") != "ssl" &&
            communicator->getProperties()->getProperty("Ice.Default.Protocol") != "wss")
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
            Ice::CommunicatorHolder comm(initData);
            comm->stringToProxy("test:" + helper->getTestEndpoint())->ice_pingAsync();
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
        adapter->refreshPublishedEndpoints();
        test(adapter->getPublishedEndpoints().size() == 1);
        test(*adapter->getPublishedEndpoints()[0] == *endpt);
        communicator->getProperties()->setProperty("PAdapter.PublishedEndpoints", "tcp -h localhost -p 12345 -t 20000");
        adapter->refreshPublishedEndpoints();
        test(adapter->getPublishedEndpoints().size() == 1);
        test(adapter->getPublishedEndpoints()[0]->toString() == "tcp -h localhost -p 12345 -t 20000");
        adapter->destroy();
        test(adapter->getPublishedEndpoints().empty());
    }
    cout << "ok" << endl;

    if (obj->ice_getConnection())
    {
        cout << "testing object adapter with bi-dir connection... " << flush;
        Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("");
        obj->ice_getConnection()->setAdapter(adapter);
        obj->ice_getConnection()->setAdapter(nullptr);
        adapter->deactivate();
        try
        {
            obj->ice_getConnection()->setAdapter(adapter);
            test(false);
        }
        catch (const Ice::ObjectAdapterDeactivatedException&)
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
        adapter->refreshPublishedEndpoints();
        test(adapter->getPublishedEndpoints().size() == 1);
        test(adapter->getPublishedEndpoints()[0]->toString() == "tcp -h localhost -p 23457 -t 30000");
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
    cout << "testing server idle time..." << flush;
    {
        InitializationData idleInitData;
        idleInitData.properties = communicator->getProperties()->clone();
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
    }
    cout << "ok" << endl;
}
