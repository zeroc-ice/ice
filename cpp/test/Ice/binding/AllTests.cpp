// Copyright (c) ZeroC, Inc.

#include "../../src/Ice/Random.h"
#include "Ice/Ice.h"
#include "Test.h"
#include "TestHelper.h"
#include <set>

#include <functional>

using namespace std;
using namespace Test;

string
getAdapterNameWithAMI(const TestIntfPrx& test)
{
    return test->getAdapterNameAsync().get();
}

TestIntfPrx
createTestIntfPrx(vector<optional<RemoteObjectAdapterPrx>>& adapters)
{
    Ice::EndpointSeq endpoints;
    optional<TestIntfPrx> test;
    for (const auto& adapter : adapters)
    {
        test = adapter->getTestIntf();
        Ice::EndpointSeq edpts = test->ice_getEndpoints();
        endpoints.insert(endpoints.end(), edpts.begin(), edpts.end());
    }
    return test->ice_endpoints(endpoints);
}

void
deactivate(const RemoteCommunicatorPrx& com, vector<optional<RemoteObjectAdapterPrx>>& adapters)
{
    for (const auto& adapter : adapters)
    {
        com->deactivateObjectAdapter(adapter);
    }
}

void
allTests(Test::TestHelper* helper)
{
    Ice::CommunicatorPtr communicator = helper->communicator();
    RemoteCommunicatorPrx com(communicator, "communicator:" + helper->getTestEndpoint());

    cout << "testing binding with single endpoint... " << flush;
    {
        optional<RemoteObjectAdapterPrx> adapter = com->createObjectAdapter("Adapter", "default");

        optional<TestIntfPrx> test1 = adapter->getTestIntf();
        optional<TestIntfPrx> test2 = adapter->getTestIntf();
        test(test1->ice_getConnection() == test2->ice_getConnection());

        test1->ice_ping();
        test2->ice_ping();

        com->deactivateObjectAdapter(adapter);

        const TestIntfPrx& test3(test1.value());
        test(test3->ice_getConnection() == test1->ice_getConnection());
        test(test3->ice_getConnection() == test2->ice_getConnection());

        try
        {
            test3->ice_ping();
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

    cout << "testing binding with multiple endpoints... " << flush;
    {
        vector<optional<RemoteObjectAdapterPrx>> adapters;
        adapters.push_back(com->createObjectAdapter("Adapter11", "default"));
        adapters.push_back(com->createObjectAdapter("Adapter12", "default"));
        adapters.push_back(com->createObjectAdapter("Adapter13", "default"));

        //
        // Ensure that when a connection is opened it's reused for new
        // proxies and that all endpoints are eventually tried.
        //
        set<string> names;
        names.insert("Adapter11");
        names.insert("Adapter12");
        names.insert("Adapter13");
        while (!names.empty())
        {
            vector<optional<RemoteObjectAdapterPrx>> adpts = adapters;

            TestIntfPrx test1 = createTestIntfPrx(adpts);
            IceInternal::shuffle(adpts.begin(), adpts.end());
            TestIntfPrx test2 = createTestIntfPrx(adpts);
            IceInternal::shuffle(adpts.begin(), adpts.end());
            TestIntfPrx test3 = createTestIntfPrx(adpts);

            test(test1->ice_getConnection() == test2->ice_getConnection());
            test(test2->ice_getConnection() == test3->ice_getConnection());

            names.erase(test1->getAdapterName());
            test1->ice_getConnection()->close().get();
        }

        //
        // Ensure that the proxy correctly caches the connection (we
        // always send the request over the same connection.)
        //
        {
            for (const auto& adapter : adapters)
            {
                adapter->getTestIntf()->ice_ping();
            }

            TestIntfPrx test = createTestIntfPrx(adapters);
            string name = test->getAdapterName();
            const int nRetry = 10;
            int i;
            for (i = 0; i < nRetry && test->getAdapterName() == name; i++)
                ;
            test(i == nRetry);

            for (const auto& adapter : adapters)
            {
                adapter->getTestIntf()->ice_getConnection()->close().get();
            }
        }

        //
        // Deactivate an adapter and ensure that we can still
        // establish the connection to the remaining adapters.
        //
        com->deactivateObjectAdapter(adapters[0]);
        names.insert("Adapter12");
        names.insert("Adapter13");
        while (!names.empty())
        {
            vector<optional<RemoteObjectAdapterPrx>> adpts = adapters;

            TestIntfPrx test1 = createTestIntfPrx(adpts);
            IceInternal::shuffle(adpts.begin(), adpts.end());
            TestIntfPrx test2 = createTestIntfPrx(adpts);
            IceInternal::shuffle(adpts.begin(), adpts.end());
            TestIntfPrx test3 = createTestIntfPrx(adpts);

            test(test1->ice_getConnection() == test2->ice_getConnection());
            test(test2->ice_getConnection() == test3->ice_getConnection());

            names.erase(test1->getAdapterName());
            test1->ice_getConnection()->close().get();
        }

        //
        // Deactivate an adapter and ensure that we can still
        // establish the connection to the remaining adapter.
        //
        com->deactivateObjectAdapter(adapters[2]);
        TestIntfPrx test = createTestIntfPrx(adapters);
        test(test->getAdapterName() == "Adapter12");

        deactivate(com, adapters);
    }
    cout << "ok" << endl;

    cout << "testing binding with multiple random endpoints... " << flush;
    {
        vector<optional<RemoteObjectAdapterPrx>> adapters;
        adapters.push_back(com->createObjectAdapter("AdapterRandom11", "default"));
        adapters.push_back(com->createObjectAdapter("AdapterRandom12", "default"));
        adapters.push_back(com->createObjectAdapter("AdapterRandom13", "default"));
        adapters.push_back(com->createObjectAdapter("AdapterRandom14", "default"));
        adapters.push_back(com->createObjectAdapter("AdapterRandom15", "default"));

        int count = 20;
        int adapterCount = static_cast<int>(adapters.size());
        while (--count > 0)
        {
#ifdef _WIN32
            if (count == 1)
            {
                com->deactivateObjectAdapter(adapters[4]);
                --adapterCount;
            }
#else
            if (count < 20 && count % 4 == 0)
            {
                com->deactivateObjectAdapter(adapters[static_cast<size_t>(count / 4 - 1)]);
                --adapterCount;
            }
#endif
            vector<optional<TestIntfPrx>> proxies;
            proxies.resize(10);
            unsigned int i;
            for (i = 0; i < proxies.size(); ++i)
            {
                vector<optional<RemoteObjectAdapterPrx>> adpts;
                adpts.resize(IceInternal::random(static_cast<unsigned int>(adapters.size())));
                if (adpts.empty())
                {
                    adpts.resize(1);
                }
                for (auto& adpt : adpts)
                {
                    adpt = adapters[IceInternal::random(static_cast<unsigned int>(adapters.size()))];
                }
                proxies[i] = createTestIntfPrx(adpts);
            }

            for (i = 0; i < proxies.size(); i++)
            {
                proxies[i]->getAdapterNameAsync(nullptr);
            }
            for (i = 0; i < proxies.size(); i++)
            {
                try
                {
                    proxies[i]->ice_ping();
                }
                catch (const Ice::LocalException&)
                {
                }
            }
            set<Ice::ConnectionPtr> connections;
            for (i = 0; i < proxies.size(); i++)
            {
                if (proxies[i]->ice_getCachedConnection())
                {
                    connections.insert(proxies[i]->ice_getCachedConnection());
                }
            }
            test(static_cast<int>(connections.size()) <= adapterCount);

            for (const auto& adapter : adapters)
            {
                try
                {
                    adapter->getTestIntf()->ice_getConnection()->close().get();
                }
                catch (const Ice::LocalException&)
                {
                    // Expected if adapter is down.
                }
            }
        }
    }
    cout << "ok" << endl;

    cout << "testing binding with multiple endpoints and AMI... " << flush;
    {
        vector<optional<RemoteObjectAdapterPrx>> adapters;
        adapters.push_back(com->createObjectAdapter("AdapterAMI11", "default"));
        adapters.push_back(com->createObjectAdapter("AdapterAMI12", "default"));
        adapters.push_back(com->createObjectAdapter("AdapterAMI13", "default"));

        //
        // Ensure that when a connection is opened it's reused for new
        // proxies and that all endpoints are eventually tried.
        //
        set<string> names;
        names.insert("AdapterAMI11");
        names.insert("AdapterAMI12");
        names.insert("AdapterAMI13");
        while (!names.empty())
        {
            vector<optional<RemoteObjectAdapterPrx>> adpts = adapters;

            TestIntfPrx test1 = createTestIntfPrx(adpts);
            IceInternal::shuffle(adpts.begin(), adpts.end());
            TestIntfPrx test2 = createTestIntfPrx(adpts);
            IceInternal::shuffle(adpts.begin(), adpts.end());
            TestIntfPrx test3 = createTestIntfPrx(adpts);

            test(test1->ice_getConnection() == test2->ice_getConnection());
            test(test2->ice_getConnection() == test3->ice_getConnection());

            names.erase(getAdapterNameWithAMI(test1));
            test1->ice_getConnection()->close().get();
        }

        //
        // Ensure that the proxy correctly caches the connection (we
        // always send the request over the same connection.)
        //
        {
            for (const auto& adapter : adapters)
            {
                adapter->getTestIntf()->ice_ping();
            }

            TestIntfPrx test = createTestIntfPrx(adapters);
            string name = getAdapterNameWithAMI(test);
            const int nRetry = 10;
            int i;
            for (i = 0; i < nRetry && getAdapterNameWithAMI(test) == name; i++)
                ;
            test(i == nRetry);

            for (const auto& adapter : adapters)
            {
                adapter->getTestIntf()->ice_getConnection()->close().get();
            }
        }

        //
        // Deactivate an adapter and ensure that we can still
        // establish the connection to the remaining adapters.
        //
        com->deactivateObjectAdapter(adapters[0]);
        names.insert("AdapterAMI12");
        names.insert("AdapterAMI13");
        while (!names.empty())
        {
            vector<optional<RemoteObjectAdapterPrx>> adpts = adapters;

            TestIntfPrx test1 = createTestIntfPrx(adpts);
            IceInternal::shuffle(adpts.begin(), adpts.end());
            TestIntfPrx test2 = createTestIntfPrx(adpts);
            IceInternal::shuffle(adpts.begin(), adpts.end());
            TestIntfPrx test3 = createTestIntfPrx(adpts);

            test(test1->ice_getConnection() == test2->ice_getConnection());
            test(test2->ice_getConnection() == test3->ice_getConnection());

            names.erase(test1->getAdapterName());
            test1->ice_getConnection()->close().get();
        }

        //
        // Deactivate an adapter and ensure that we can still
        // establish the connection to the remaining adapter.
        //
        com->deactivateObjectAdapter(adapters[2]);
        TestIntfPrx test = createTestIntfPrx(adapters);
        test(test->getAdapterName() == "AdapterAMI12");

        deactivate(com, adapters);
    }
    cout << "ok" << endl;

    cout << "testing random endpoint selection... " << flush;
    {
        vector<optional<RemoteObjectAdapterPrx>> adapters;
        adapters.push_back(com->createObjectAdapter("Adapter21", "default"));
        adapters.push_back(com->createObjectAdapter("Adapter22", "default"));
        adapters.push_back(com->createObjectAdapter("Adapter23", "default"));

        TestIntfPrx test = createTestIntfPrx(adapters);
        test(test->ice_getEndpointSelection() == Ice::EndpointSelectionType::Random);

        set<string> names;
        names.insert("Adapter21");
        names.insert("Adapter22");
        names.insert("Adapter23");
        while (!names.empty())
        {
            names.erase(test->getAdapterName());
            test->ice_getConnection()->close().get();
        }

        test = test->ice_endpointSelection(Ice::EndpointSelectionType::Random);
        test(test->ice_getEndpointSelection() == Ice::EndpointSelectionType::Random);

        names.insert("Adapter21");
        names.insert("Adapter22");
        names.insert("Adapter23");
        while (!names.empty())
        {
            names.erase(test->getAdapterName());
            test->ice_getConnection()->close().get();
        }

        deactivate(com, adapters);
    }
    cout << "ok" << endl;

    cout << "testing ordered endpoint selection... " << flush;
    {
        vector<optional<RemoteObjectAdapterPrx>> adapters;
        adapters.push_back(com->createObjectAdapter("Adapter31", "default"));
        adapters.push_back(com->createObjectAdapter("Adapter32", "default"));
        adapters.push_back(com->createObjectAdapter("Adapter33", "default"));

        TestIntfPrx test = createTestIntfPrx(adapters);
        test = test->ice_endpointSelection(Ice::EndpointSelectionType::Ordered);
        test(test->ice_getEndpointSelection() == Ice::EndpointSelectionType::Ordered);
        const int nRetry = 5;
        int i;

        //
        // Ensure that endpoints are tried in order by deactivating the adapters
        // one after the other.
        //
        for (i = 0; i < nRetry && test->getAdapterName() == "Adapter31"; i++)
            ;
#if TARGET_OS_IPHONE > 0
        if (i != nRetry)
        {
            test->ice_getConnection()->close().get();
            for (i = 0; i < nRetry && test->getAdapterName() == "Adapter31"; i++)
                ;
        }
#endif
        test(i == nRetry);
        com->deactivateObjectAdapter(adapters[0]);
        for (i = 0; i < nRetry && test->getAdapterName() == "Adapter32"; i++)
            ;
#if TARGET_OS_IPHONE > 0
        if (i != nRetry)
        {
            test->ice_getConnection()->close().get();
            for (i = 0; i < nRetry && test->getAdapterName() == "Adapter32"; i++)
                ;
        }
#endif
        test(i == nRetry);
        com->deactivateObjectAdapter(adapters[1]);
        for (i = 0; i < nRetry && test->getAdapterName() == "Adapter33"; i++)
            ;
#if TARGET_OS_IPHONE > 0
        if (i != nRetry)
        {
            test->ice_getConnection()->close().get();
            for (i = 0; i < nRetry && test->getAdapterName() == "Adapter33"; i++)
                ;
        }
#endif
        test(i == nRetry);
        com->deactivateObjectAdapter(adapters[2]);

        try
        {
            test->getAdapterName();
        }
        catch (const Ice::ConnectFailedException&)
        {
        }
        catch (const Ice::ConnectTimeoutException&)
        {
        }
        Ice::EndpointSeq endpoints = test->ice_getEndpoints();

        adapters.clear();

        //
        // Now, re-activate the adapters with the same endpoints in the opposite
        // order.
        //
        adapters.push_back(com->createObjectAdapter("Adapter36", endpoints[2]->toString()));
        for (i = 0; i < nRetry && test->getAdapterName() == "Adapter36"; i++)
            ;
#if TARGET_OS_IPHONE > 0
        if (i != nRetry)
        {
            test->ice_getConnection()->close().get();
            for (i = 0; i < nRetry && test->getAdapterName() == "Adapter36"; i++)
                ;
        }
#endif
        test(i == nRetry);
        test->ice_getConnection()->close().get();
        adapters.push_back(com->createObjectAdapter("Adapter35", endpoints[1]->toString()));
        for (i = 0; i < nRetry && test->getAdapterName() == "Adapter35"; i++)
            ;
#if TARGET_OS_IPHONE > 0
        if (i != nRetry)
        {
            test->ice_getConnection()->close().get();
            for (i = 0; i < nRetry && test->getAdapterName() == "Adapter35"; i++)
                ;
        }
#endif
        test(i == nRetry);
        test->ice_getConnection()->close().get();
        adapters.push_back(com->createObjectAdapter("Adapter34", endpoints[0]->toString()));
        for (i = 0; i < nRetry && test->getAdapterName() == "Adapter34"; i++)
            ;
#if TARGET_OS_IPHONE > 0
        if (i != nRetry)
        {
            test->ice_getConnection()->close().get();
            for (i = 0; i < nRetry && test->getAdapterName() == "Adapter34"; i++)
                ;
        }
#endif
        test(i == nRetry);

        deactivate(com, adapters);
    }
    cout << "ok" << endl;

    cout << "testing per request binding with single endpoint... " << flush;
    {
        optional<RemoteObjectAdapterPrx> adapter = com->createObjectAdapter("Adapter41", "default");

        TestIntfPrx test1 = adapter->getTestIntf()->ice_connectionCached(false);
        TestIntfPrx test2 = adapter->getTestIntf()->ice_connectionCached(false);
        test(!test1->ice_isConnectionCached());
        test(!test2->ice_isConnectionCached());
        test(test1->ice_getConnection() && test2->ice_getConnection());
        test(test1->ice_getConnection() == test2->ice_getConnection());

        test1->ice_ping();

        com->deactivateObjectAdapter(adapter);

        const TestIntfPrx& test3(test1);
        try
        {
            test(test3->ice_getConnection() == test1->ice_getConnection());
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

    cout << "testing per request binding with multiple endpoints... " << flush;
    {
        vector<optional<RemoteObjectAdapterPrx>> adapters;
        adapters.push_back(com->createObjectAdapter("Adapter51", "default"));
        adapters.push_back(com->createObjectAdapter("Adapter52", "default"));
        adapters.push_back(com->createObjectAdapter("Adapter53", "default"));

        auto test = createTestIntfPrx(adapters)->ice_connectionCached(false);
        test(!test->ice_isConnectionCached());

        set<string> names;
        names.insert("Adapter51");
        names.insert("Adapter52");
        names.insert("Adapter53");
        while (!names.empty())
        {
            names.erase(test->getAdapterName());
        }

        com->deactivateObjectAdapter(adapters[0]);

        names.insert("Adapter52");
        names.insert("Adapter53");
        while (!names.empty())
        {
            names.erase(test->getAdapterName());
        }

        com->deactivateObjectAdapter(adapters[2]);

        test(test->getAdapterName() == "Adapter52");

        deactivate(com, adapters);
    }
    cout << "ok" << endl;

    cout << "testing per request binding with multiple endpoints and AMI... " << flush;
    {
        vector<optional<RemoteObjectAdapterPrx>> adapters;
        adapters.push_back(com->createObjectAdapter("AdapterAMI51", "default"));
        adapters.push_back(com->createObjectAdapter("AdapterAMI52", "default"));
        adapters.push_back(com->createObjectAdapter("AdapterAMI53", "default"));

        TestIntfPrx test = createTestIntfPrx(adapters)->ice_connectionCached(false);
        test(!test->ice_isConnectionCached());

        set<string> names;
        names.insert("AdapterAMI51");
        names.insert("AdapterAMI52");
        names.insert("AdapterAMI53");
        while (!names.empty())
        {
            names.erase(getAdapterNameWithAMI(test));
        }

        com->deactivateObjectAdapter(adapters[0]);

        names.insert("AdapterAMI52");
        names.insert("AdapterAMI53");
        while (!names.empty())
        {
            names.erase(getAdapterNameWithAMI(test));
        }

        com->deactivateObjectAdapter(adapters[2]);

        test(test->getAdapterName() == "AdapterAMI52");

        deactivate(com, adapters);
    }
    cout << "ok" << endl;

    cout << "testing per request binding and ordered endpoint selection... " << flush;
    {
        vector<optional<RemoteObjectAdapterPrx>> adapters;
        adapters.push_back(com->createObjectAdapter("Adapter61", "default"));
        adapters.push_back(com->createObjectAdapter("Adapter62", "default"));
        adapters.push_back(com->createObjectAdapter("Adapter63", "default"));

        TestIntfPrx test = createTestIntfPrx(adapters);
        test = test->ice_endpointSelection(Ice::EndpointSelectionType::Ordered);
        test(test->ice_getEndpointSelection() == Ice::EndpointSelectionType::Ordered);
        test = test->ice_connectionCached(false);
        test(!test->ice_isConnectionCached());
        const int nRetry = 5;
        int i;

        //
        // Ensure that endpoints are tried in order by deactivating the adapters
        // one after the other.
        //
        for (i = 0; i < nRetry && test->getAdapterName() == "Adapter61"; i++)
            ;
#if TARGET_OS_IPHONE > 0
        test(i >= nRetry - 1); // WORKAROUND: for connection establishment hang.
#else
        test(i == nRetry);
#endif
        com->deactivateObjectAdapter(adapters[0]);
        for (i = 0; i < nRetry && test->getAdapterName() == "Adapter62"; i++)
            ;
#if TARGET_OS_IPHONE > 0
        test(i >= nRetry - 1); // WORKAROUND: for connection establishment hang.
#else
        test(i == nRetry);
#endif
        com->deactivateObjectAdapter(adapters[1]);
        for (i = 0; i < nRetry && test->getAdapterName() == "Adapter63"; i++)
            ;
#if TARGET_OS_IPHONE > 0
        test(i >= nRetry - 1); // WORKAROUND: for connection establishment hang.
#else
        test(i == nRetry);
#endif
        com->deactivateObjectAdapter(adapters[2]);

        try
        {
            test->getAdapterName();
        }
        catch (const Ice::ConnectFailedException&)
        {
        }
        catch (const Ice::ConnectTimeoutException&)
        {
        }
        Ice::EndpointSeq endpoints = test->ice_getEndpoints();

        adapters.clear();

        //
        // Now, re-activate the adapters with the same endpoints in the opposite
        // order.
        //
        adapters.push_back(com->createObjectAdapter("Adapter66", endpoints[2]->toString()));
        for (i = 0; i < nRetry && test->getAdapterName() == "Adapter66"; i++)
            ;
#if TARGET_OS_IPHONE > 0
        test(i >= nRetry - 1); // WORKAROUND: for connection establishment hang.
#else
        test(i == nRetry);
#endif
        adapters.push_back(com->createObjectAdapter("Adapter65", endpoints[1]->toString()));
        for (i = 0; i < nRetry && test->getAdapterName() == "Adapter65"; i++)
            ;
#if TARGET_OS_IPHONE > 0
        test(i >= nRetry - 1); // WORKAROUND: for connection establishment hang.
#else
        test(i == nRetry);
#endif
        adapters.push_back(com->createObjectAdapter("Adapter64", endpoints[0]->toString()));
        for (i = 0; i < nRetry && test->getAdapterName() == "Adapter64"; i++)
            ;
#if TARGET_OS_IPHONE > 0
        test(i >= nRetry - 1); // WORKAROUND: for connection establishment hang.
#else
        test(i == nRetry);
#endif

        deactivate(com, adapters);
    }
    cout << "ok" << endl;

    cout << "testing per request binding and ordered endpoint selection and AMI... " << flush;
    {
        vector<optional<RemoteObjectAdapterPrx>> adapters;
        adapters.push_back(com->createObjectAdapter("AdapterAMI61", "default"));
        adapters.push_back(com->createObjectAdapter("AdapterAMI62", "default"));
        adapters.push_back(com->createObjectAdapter("AdapterAMI63", "default"));

        TestIntfPrx test = createTestIntfPrx(adapters);
        test = test->ice_endpointSelection(Ice::EndpointSelectionType::Ordered);
        test(test->ice_getEndpointSelection() == Ice::EndpointSelectionType::Ordered);
        test = test->ice_connectionCached(false);
        test(!test->ice_isConnectionCached());
        const int nRetry = 5;
        int i;

        //
        // Ensure that endpoints are tried in order by deactivating the adapters
        // one after the other.
        //
        for (i = 0; i < nRetry && getAdapterNameWithAMI(test) == "AdapterAMI61"; i++)
            ;
#if TARGET_OS_IPHONE > 0
        test(i >= nRetry - 1); // WORKAROUND: for connection establishment hang.
#else
        test(i == nRetry);
#endif
        com->deactivateObjectAdapter(adapters[0]);
        for (i = 0; i < nRetry && getAdapterNameWithAMI(test) == "AdapterAMI62"; i++)
            ;
#if TARGET_OS_IPHONE > 0
        test(i >= nRetry - 1); // WORKAROUND: for connection establishment hang.
#else
        test(i == nRetry);
#endif
        com->deactivateObjectAdapter(adapters[1]);
        for (i = 0; i < nRetry && getAdapterNameWithAMI(test) == "AdapterAMI63"; i++)
            ;
#if TARGET_OS_IPHONE > 0
        test(i >= nRetry - 1); // WORKAROUND: for connection establishment hang.
#else
        test(i == nRetry);
#endif
        com->deactivateObjectAdapter(adapters[2]);

        try
        {
            test->getAdapterName();
        }
        catch (const Ice::ConnectFailedException&)
        {
        }
        catch (const Ice::ConnectTimeoutException&)
        {
        }

        Ice::EndpointSeq endpoints = test->ice_getEndpoints();

        adapters.clear();

        //
        // Now, re-activate the adapters with the same endpoints in the opposite
        // order.
        //
        adapters.push_back(com->createObjectAdapter("AdapterAMI66", endpoints[2]->toString()));
        for (i = 0; i < nRetry && getAdapterNameWithAMI(test) == "AdapterAMI66"; i++)
            ;
#if TARGET_OS_IPHONE > 0
        test(i >= nRetry - 1); // WORKAROUND: for connection establishment hang.
#else
        test(i == nRetry);
#endif
        adapters.push_back(com->createObjectAdapter("AdapterAMI65", endpoints[1]->toString()));
        for (i = 0; i < nRetry && getAdapterNameWithAMI(test) == "AdapterAMI65"; i++)
            ;
        test(i == nRetry);
        adapters.push_back(com->createObjectAdapter("AdapterAMI64", endpoints[0]->toString()));
        for (i = 0; i < nRetry && getAdapterNameWithAMI(test) == "AdapterAMI64"; i++)
            ;
        test(i == nRetry);

        deactivate(com, adapters);
    }
    cout << "ok" << endl;

    cout << "testing endpoint mode filtering... " << flush;
    {
        vector<optional<RemoteObjectAdapterPrx>> adapters;
        adapters.push_back(com->createObjectAdapter("Adapter71", "default"));
        adapters.push_back(com->createObjectAdapter("Adapter72", "udp"));

        TestIntfPrx test = createTestIntfPrx(adapters);
        test(test->getAdapterName() == "Adapter71");

        TestIntfPrx testUDP = test->ice_datagram();
        test(test->ice_getConnection() != testUDP->ice_getConnection());
        try
        {
            testUDP->getAdapterName();
        }
        catch (const Ice::TwowayOnlyException&)
        {
        }
    }
    cout << "ok" << endl;

    {
        cout << "testing ipv4 & ipv6 connections... " << flush;

        Ice::PropertiesPtr ipv4 = Ice::createProperties();
        ipv4->setProperty("Ice.IPv4", "1");
        ipv4->setProperty("Ice.IPv6", "0");
        ipv4->setProperty("Adapter.Endpoints", "tcp -h localhost");

        Ice::PropertiesPtr ipv6 = Ice::createProperties();
        ipv6->setProperty("Ice.IPv4", "0");
        ipv6->setProperty("Ice.IPv6", "1");
        ipv6->setProperty("Adapter.Endpoints", "tcp -h localhost");

        Ice::PropertiesPtr bothPreferIPv4 = Ice::createProperties();
        bothPreferIPv4->setProperty("Ice.IPv4", "1");
        bothPreferIPv4->setProperty("Ice.IPv6", "1");
        bothPreferIPv4->setProperty("Ice.PreferIPv6Address", "0");
        bothPreferIPv4->setProperty("Adapter.Endpoints", "tcp -h localhost");

        Ice::PropertiesPtr bothPreferIPv6 = Ice::createProperties();
        bothPreferIPv6->setProperty("Ice.IPv4", "1");
        bothPreferIPv6->setProperty("Ice.IPv6", "1");
        bothPreferIPv6->setProperty("Ice.PreferIPv6Address", "1");
        bothPreferIPv6->setProperty("Adapter.Endpoints", "tcp -h localhost");

        vector<Ice::PropertiesPtr> clientProps;
        clientProps.push_back(ipv4);
        clientProps.push_back(ipv6);
        clientProps.push_back(bothPreferIPv4);
        clientProps.push_back(bothPreferIPv6);

        string endpoint;
        {
            ostringstream str;
            str << "tcp -p " << helper->getTestPort(2);
            endpoint = str.str();
        }

        Ice::PropertiesPtr anyipv4 = ipv4->clone();
        anyipv4->setProperty("Adapter.Endpoints", endpoint);
        anyipv4->setProperty("Adapter.PublishedEndpoints", endpoint + " -h 127.0.0.1");

        Ice::PropertiesPtr anyipv6 = ipv6->clone();
        anyipv6->setProperty("Adapter.Endpoints", endpoint);
        anyipv6->setProperty("Adapter.PublishedEndpoints", endpoint + " -h \"::1\"");

        Ice::PropertiesPtr anyboth = Ice::createProperties();
        anyboth->setProperty("Ice.IPv4", "1");
        anyboth->setProperty("Ice.IPv6", "1");
        anyboth->setProperty("Adapter.Endpoints", endpoint);
        anyboth->setProperty("Adapter.PublishedEndpoints", endpoint + " -h 127.0.0.1:" + endpoint + " -h \"::1\"");

        Ice::PropertiesPtr localipv4 = ipv4->clone();
        localipv4->setProperty("Adapter.Endpoints", "tcp -h 127.0.0.1");

        Ice::PropertiesPtr localipv6 = ipv6->clone();
        localipv6->setProperty("Adapter.Endpoints", "tcp -h \"::1\"");

        vector<Ice::PropertiesPtr> serverProps = clientProps;
        serverProps.push_back(anyipv4);
        serverProps.push_back(anyipv6);
        serverProps.push_back(anyboth);
        serverProps.push_back(localipv4);
        serverProps.push_back(localipv6);

        bool ipv6NotSupported = false;
        for (const auto& serverProp : serverProps)
        {
            Ice::InitializationData serverInitData;
            serverInitData.properties = serverProp;
            Ice::CommunicatorPtr serverCommunicator = Ice::initialize(serverInitData);
            Ice::ObjectAdapterPtr oa;
            try
            {
                oa = serverCommunicator->createObjectAdapter("Adapter");
                oa->activate();
            }
            catch (const Ice::DNSException&)
            {
                serverCommunicator->destroy();
                continue; // IP version not supported.
            }
            catch (const Ice::SocketException&)
            {
                if (serverProp == ipv6)
                {
                    ipv6NotSupported = true;
                }
                serverCommunicator->destroy();
                continue; // IP version not supported.
            }

            // Ensure the published endpoints are actually valid. On
            // Fedora, binding to "localhost" with IPv6 only works but
            // resolving localhost don't return the IPv6 address.
            Ice::ObjectPrx prx = oa->createProxy(Ice::stringToIdentity("dummy"));
            try
            {
                prx->ice_collocationOptimized(false)->ice_ping();
            }
            catch (const Ice::LocalException&)
            {
                serverCommunicator->destroy();
                continue; // IP version not supported.
            }

            string strPrx = prx->ice_toString();
            for (const auto& clientProp : clientProps)
            {
                Ice::InitializationData clientInitData;
                clientInitData.properties = clientProp;
                Ice::CommunicatorHolder clientCommunicator(clientInitData);
                Ice::ObjectPrx clientPrx(clientCommunicator.communicator(), strPrx);
                try
                {
                    clientPrx->ice_ping();
                    test(false);
                }
                catch (const Ice::ObjectNotExistException&)
                {
                    // Expected, no object registered.
                }
                catch (const Ice::DNSException&)
                {
                    // Expected if no IPv4 or IPv6 address is
                    // associated to localhost or if trying to connect
                    // to an any endpoint with the wrong IP version,
                    // e.g.: resolving an IPv4 address when only IPv6
                    // is enabled fails with a DNS exception.
                }
                catch (const Ice::SocketException&)
                {
                    test(
                        (serverProp == ipv4 && clientProp == ipv6) || (serverProp == ipv6 && clientProp == ipv4) ||
                        (serverProp == bothPreferIPv4 && clientProp == ipv6) ||
                        (serverProp == bothPreferIPv6 && clientProp == ipv4) ||
                        (serverProp == bothPreferIPv6 && clientProp == ipv6 && ipv6NotSupported) ||
                        (serverProp == anyipv4 && clientProp == ipv6) ||
                        (serverProp == anyipv6 && clientProp == ipv4) ||
                        (serverProp == localipv4 && clientProp == ipv6) ||
                        (serverProp == localipv6 && clientProp == ipv4) ||
                        (serverProp == ipv6 && clientProp == bothPreferIPv4) ||
                        (serverProp == ipv6 && clientProp == bothPreferIPv6) ||
                        (serverProp == bothPreferIPv6 && clientProp == ipv6));
                }
            }
            serverCommunicator->destroy();
        }

        cout << "ok" << endl;
    }
    com->shutdown();
}
