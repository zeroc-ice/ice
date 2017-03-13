// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Random.h>
#include <Ice/Ice.h>
#include <TestCommon.h>
#include <Test.h>
#include <set>

#include <functional>

using namespace std;
using namespace Test;

struct RandomNumberGenerator : public std::unary_function<ptrdiff_t, ptrdiff_t>
{
    ptrdiff_t operator()(ptrdiff_t d)
    {
        return IceUtilInternal::random(static_cast<int>(d));
    }
};

#ifndef ICE_CPP11_MAPPING
class GetAdapterNameCB : public IceUtil::Shared, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    void
    response(const string& name)
    {
        Lock sync(*this);
        assert(!name.empty());
        _name = name;
        notify();
    }

    void
    exception(const Ice::Exception&)
    {
        test(false);
    }

    virtual string
    getResult()
    {
        Lock sync(*this);
        while(_name.empty())
        {
            wait();
        }
        return _name;
    }

private:

    string _name;
};
typedef IceUtil::Handle<GetAdapterNameCB> GetAdapterNameCBPtr;
#endif

string
getAdapterNameWithAMI(const TestIntfPrxPtr& test)
{
#ifdef ICE_CPP11_MAPPING
    return test->getAdapterNameAsync().get();
#else
    GetAdapterNameCBPtr cb = new GetAdapterNameCB();
    test->begin_getAdapterName(
        newCallback_TestIntf_getAdapterName(cb, &GetAdapterNameCB::response,  &GetAdapterNameCB::exception));
    return cb->getResult();
#endif
}

TestIntfPrxPtr
createTestIntfPrx(vector<RemoteObjectAdapterPrxPtr>& adapters)
{
    Ice::EndpointSeq endpoints;
    TestIntfPrxPtr test;
    for(vector<RemoteObjectAdapterPrxPtr>::const_iterator p = adapters.begin(); p != adapters.end(); ++p)
    {
        test = (*p)->getTestIntf();
        Ice::EndpointSeq edpts = test->ice_getEndpoints();
        endpoints.insert(endpoints.end(), edpts.begin(), edpts.end());
    }
    return ICE_UNCHECKED_CAST(TestIntfPrx, test->ice_endpoints(endpoints));
}

void
deactivate(const RemoteCommunicatorPrxPtr& com, vector<RemoteObjectAdapterPrxPtr>& adapters)
{
    for(vector<RemoteObjectAdapterPrxPtr>::const_iterator p = adapters.begin(); p != adapters.end(); ++p)
    {
        com->deactivateObjectAdapter(*p);
    }
}

void
allTests(const Ice::CommunicatorPtr& communicator)
{
    string ref = "communicator:" + getTestEndpoint(communicator, 0);
    RemoteCommunicatorPrxPtr com = ICE_UNCHECKED_CAST(RemoteCommunicatorPrx, communicator->stringToProxy(ref));

    RandomNumberGenerator rng;

	cout << "testing binding with single endpoint... " << flush;
    {
        RemoteObjectAdapterPrxPtr adapter = com->createObjectAdapter("Adapter", "default");

        TestIntfPrxPtr test1 = adapter->getTestIntf();
        TestIntfPrxPtr test2 = adapter->getTestIntf();
        test(test1->ice_getConnection() == test2->ice_getConnection());

        test1->ice_ping();
        test2->ice_ping();

        com->deactivateObjectAdapter(adapter);

        TestIntfPrxPtr test3 = ICE_UNCHECKED_CAST(TestIntfPrx, test1);
        test(test3->ice_getConnection() == test1->ice_getConnection());
        test(test3->ice_getConnection() == test2->ice_getConnection());

        try
        {
            test3->ice_ping();
            test(false);
        }
        catch(const Ice::ConnectFailedException&)
        {
        }
#ifdef _WIN32
        catch(const Ice::ConnectTimeoutException&)
        {
        }
#endif
    }
    cout << "ok" << endl;

    cout << "testing binding with multiple endpoints... " << flush;
    {
        vector<RemoteObjectAdapterPrxPtr> adapters;
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
        while(!names.empty())
        {
            vector<RemoteObjectAdapterPrxPtr> adpts = adapters;

            TestIntfPrxPtr test1 = createTestIntfPrx(adpts);
            random_shuffle(adpts.begin(), adpts.end(), rng);
            TestIntfPrxPtr test2 = createTestIntfPrx(adpts);
            random_shuffle(adpts.begin(), adpts.end(), rng);
            TestIntfPrxPtr test3 = createTestIntfPrx(adpts);

            test(test1->ice_getConnection() == test2->ice_getConnection());
            test(test2->ice_getConnection() == test3->ice_getConnection());

            names.erase(test1->getAdapterName());
            test1->ice_getConnection()->close(Ice::ICE_SCOPED_ENUM(ConnectionClose, GracefullyWithWait));
        }

        //
        // Ensure that the proxy correctly caches the connection (we
        // always send the request over the same connection.)
        //
        {
            for(vector<RemoteObjectAdapterPrxPtr>::const_iterator p = adapters.begin(); p != adapters.end(); ++p)
            {
                (*p)->getTestIntf()->ice_ping();
            }

            TestIntfPrxPtr test = createTestIntfPrx(adapters);
            string name = test->getAdapterName();
            const int nRetry = 10;
            int i;
            for(i = 0; i < nRetry &&  test->getAdapterName() == name; i++);
            test(i == nRetry);

            for(vector<RemoteObjectAdapterPrxPtr>::const_iterator q = adapters.begin(); q != adapters.end(); ++q)
            {
                (*q)->getTestIntf()->ice_getConnection()->close(
                    Ice::ICE_SCOPED_ENUM(ConnectionClose, GracefullyWithWait));
            }
        }

        //
        // Deactivate an adapter and ensure that we can still
        // establish the connection to the remaining adapters.
        //
        com->deactivateObjectAdapter(adapters[0]);
        names.insert("Adapter12");
        names.insert("Adapter13");
        while(!names.empty())
        {
            vector<RemoteObjectAdapterPrxPtr> adpts = adapters;

            TestIntfPrxPtr test1 = createTestIntfPrx(adpts);
            random_shuffle(adpts.begin(), adpts.end(), rng);
            TestIntfPrxPtr test2 = createTestIntfPrx(adpts);
            random_shuffle(adpts.begin(), adpts.end(), rng);
            TestIntfPrxPtr test3 = createTestIntfPrx(adpts);

            test(test1->ice_getConnection() == test2->ice_getConnection());
            test(test2->ice_getConnection() == test3->ice_getConnection());

            names.erase(test1->getAdapterName());
            test1->ice_getConnection()->close(Ice::ICE_SCOPED_ENUM(ConnectionClose, GracefullyWithWait));
        }

        //
        // Deactivate an adapter and ensure that we can still
        // establish the connection to the remaining adapter.
        //
        com->deactivateObjectAdapter(adapters[2]);
        TestIntfPrxPtr test = createTestIntfPrx(adapters);
        test(test->getAdapterName() == "Adapter12");

        deactivate(com, adapters);
    }
    cout << "ok" << endl;

    cout << "testing binding with multiple random endpoints... " << flush;
    {
        vector<RemoteObjectAdapterPrxPtr> adapters;
        adapters.push_back(com->createObjectAdapter("AdapterRandom11", "default"));
        adapters.push_back(com->createObjectAdapter("AdapterRandom12", "default"));
        adapters.push_back(com->createObjectAdapter("AdapterRandom13", "default"));
        adapters.push_back(com->createObjectAdapter("AdapterRandom14", "default"));
        adapters.push_back(com->createObjectAdapter("AdapterRandom15", "default"));

#ifdef _WIN32
        int count = 20;
#else
        int count = 60;
#endif
        int adapterCount = static_cast<int>(adapters.size());
        while(--count > 0)
        {
#ifdef _WIN32
            if(count == 1)
            {
                com->deactivateObjectAdapter(adapters[4]);
                --adapterCount;
            }
            vector<TestIntfPrxPtr> proxies;
            proxies.resize(10);
#else
            if(count < 60 && count % 10 == 0)
            {
                com->deactivateObjectAdapter(adapters[count / 10 - 1]);
                --adapterCount;
            }
            vector<TestIntfPrxPtr> proxies;
            proxies.resize(40);
#endif
            unsigned int i;
            for(i = 0; i < proxies.size(); ++i)
            {
                vector<RemoteObjectAdapterPrxPtr> adpts;
                adpts.resize(IceUtilInternal::random(static_cast<int>(adapters.size())));
                if(adpts.empty())
                {
                    adpts.resize(1);
                }
                for(vector<RemoteObjectAdapterPrxPtr>::iterator p = adpts.begin(); p != adpts.end(); ++p)
                {
                    *p = adapters[IceUtilInternal::random(static_cast<int>(adapters.size()))];
                }
                proxies[i] = createTestIntfPrx(adpts);
            }

            for(i = 0; i < proxies.size(); i++)
            {
#ifdef ICE_CPP11_MAPPING
                proxies[i]->getAdapterNameAsync();
#else
                proxies[i]->begin_getAdapterName();
#endif
            }
            for(i = 0; i < proxies.size(); i++)
            {
                try
                {
                    proxies[i]->ice_ping();
                }
                catch(const Ice::LocalException&)
                {
                }
            }
            set<Ice::ConnectionPtr> connections;
            for(i = 0; i < proxies.size(); i++)
            {
                if(proxies[i]->ice_getCachedConnection())
                {
                    connections.insert(proxies[i]->ice_getCachedConnection());
                }
            }
            test(static_cast<int>(connections.size()) <= adapterCount);

            for(vector<RemoteObjectAdapterPrxPtr>::const_iterator q = adapters.begin(); q != adapters.end(); ++q)
            {
                try
                {
                    (*q)->getTestIntf()->ice_getConnection()->close(
                        Ice::ICE_SCOPED_ENUM(ConnectionClose, GracefullyWithWait));
                }
                catch(const Ice::LocalException&)
                {
                    // Expected if adapter is down.
                }
            }
        }
    }
    cout << "ok" << endl;

    cout << "testing binding with multiple endpoints and AMI... " << flush;
    {
        vector<RemoteObjectAdapterPrxPtr> adapters;
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
        while(!names.empty())
        {
            vector<RemoteObjectAdapterPrxPtr> adpts = adapters;

            TestIntfPrxPtr test1 = createTestIntfPrx(adpts);
            random_shuffle(adpts.begin(), adpts.end(), rng);
            TestIntfPrxPtr test2 = createTestIntfPrx(adpts);
            random_shuffle(adpts.begin(), adpts.end(), rng);
            TestIntfPrxPtr test3 = createTestIntfPrx(adpts);

            test(test1->ice_getConnection() == test2->ice_getConnection());
            test(test2->ice_getConnection() == test3->ice_getConnection());

            names.erase(getAdapterNameWithAMI(test1));
            test1->ice_getConnection()->close(Ice::ICE_SCOPED_ENUM(ConnectionClose, GracefullyWithWait));
        }

        //
        // Ensure that the proxy correctly caches the connection (we
        // always send the request over the same connection.)
        //
        {
            for(vector<RemoteObjectAdapterPrxPtr>::const_iterator p = adapters.begin(); p != adapters.end(); ++p)
            {
                (*p)->getTestIntf()->ice_ping();
            }

            TestIntfPrxPtr test = createTestIntfPrx(adapters);
            string name = getAdapterNameWithAMI(test);
            const int nRetry = 10;
            int i;
            for(i = 0; i < nRetry && getAdapterNameWithAMI(test) == name; i++);
            test(i == nRetry);

            for(vector<RemoteObjectAdapterPrxPtr>::const_iterator q = adapters.begin(); q != adapters.end(); ++q)
            {
                (*q)->getTestIntf()->ice_getConnection()->close(
                    Ice::ICE_SCOPED_ENUM(ConnectionClose, GracefullyWithWait));
            }
        }

        //
        // Deactivate an adapter and ensure that we can still
        // establish the connection to the remaining adapters.
        //
        com->deactivateObjectAdapter(adapters[0]);
        names.insert("AdapterAMI12");
        names.insert("AdapterAMI13");
        while(!names.empty())
        {
            vector<RemoteObjectAdapterPrxPtr> adpts = adapters;

            TestIntfPrxPtr test1 = createTestIntfPrx(adpts);
            random_shuffle(adpts.begin(), adpts.end(), rng);
            TestIntfPrxPtr test2 = createTestIntfPrx(adpts);
            random_shuffle(adpts.begin(), adpts.end(), rng);
            TestIntfPrxPtr test3 = createTestIntfPrx(adpts);

            test(test1->ice_getConnection() == test2->ice_getConnection());
            test(test2->ice_getConnection() == test3->ice_getConnection());

            names.erase(test1->getAdapterName());
            test1->ice_getConnection()->close(Ice::ICE_SCOPED_ENUM(ConnectionClose, GracefullyWithWait));
        }

        //
        // Deactivate an adapter and ensure that we can still
        // establish the connection to the remaining adapter.
        //
        com->deactivateObjectAdapter(adapters[2]);
        TestIntfPrxPtr test = createTestIntfPrx(adapters);
        test(test->getAdapterName() == "AdapterAMI12");

        deactivate(com, adapters);
    }
    cout << "ok" << endl;

    cout << "testing random endpoint selection... " << flush;
    {
        vector<RemoteObjectAdapterPrxPtr> adapters;
        adapters.push_back(com->createObjectAdapter("Adapter21", "default"));
        adapters.push_back(com->createObjectAdapter("Adapter22", "default"));
        adapters.push_back(com->createObjectAdapter("Adapter23", "default"));

        TestIntfPrxPtr test = createTestIntfPrx(adapters);
        test(test->ice_getEndpointSelection() == Ice::ICE_ENUM(EndpointSelectionType, Random));

        set<string> names;
        names.insert("Adapter21");
        names.insert("Adapter22");
        names.insert("Adapter23");
        while(!names.empty())
        {
            names.erase(test->getAdapterName());
            test->ice_getConnection()->close(Ice::ICE_SCOPED_ENUM(ConnectionClose, GracefullyWithWait));
        }

        test = ICE_UNCHECKED_CAST(TestIntfPrx, test->ice_endpointSelection(Ice::ICE_ENUM(EndpointSelectionType, Random)));
        test(test->ice_getEndpointSelection() == Ice::ICE_ENUM(EndpointSelectionType, Random));

        names.insert("Adapter21");
        names.insert("Adapter22");
        names.insert("Adapter23");
        while(!names.empty())
        {
            names.erase(test->getAdapterName());
            test->ice_getConnection()->close(Ice::ICE_SCOPED_ENUM(ConnectionClose, GracefullyWithWait));
        }

        deactivate(com, adapters);
    }
    cout << "ok" << endl;

    cout << "testing ordered endpoint selection... " << flush;
    {
        vector<RemoteObjectAdapterPrxPtr> adapters;
        adapters.push_back(com->createObjectAdapter("Adapter31", "default"));
        adapters.push_back(com->createObjectAdapter("Adapter32", "default"));
        adapters.push_back(com->createObjectAdapter("Adapter33", "default"));

        TestIntfPrxPtr test = createTestIntfPrx(adapters);
        test = ICE_UNCHECKED_CAST(TestIntfPrx, test->ice_endpointSelection(Ice::ICE_ENUM(EndpointSelectionType, Ordered)));
        test(test->ice_getEndpointSelection() == Ice::ICE_ENUM(EndpointSelectionType, Ordered));
        const int nRetry = 5;
        int i;

        //
        // Ensure that endpoints are tried in order by deactiving the adapters
        // one after the other.
        //
        for(i = 0; i < nRetry && test->getAdapterName() == "Adapter31"; i++);
#if TARGET_OS_IPHONE > 0
        if(i != nRetry)
        {
            test->ice_getConnection()->close(Ice::ICE_SCOPED_ENUM(ConnectionClose, GracefullyWithWait));
            for(i = 0; i < nRetry && test->getAdapterName() == "Adapter31"; i++);
        }
#endif
        test(i == nRetry);
        com->deactivateObjectAdapter(adapters[0]);
        for(i = 0; i < nRetry && test->getAdapterName() == "Adapter32"; i++);
#if TARGET_OS_IPHONE > 0
        if(i != nRetry)
        {
            test->ice_getConnection()->close(Ice::ICE_SCOPED_ENUM(ConnectionClose, GracefullyWithWait));
            for(i = 0; i < nRetry && test->getAdapterName() == "Adapter32"; i++);
        }
#endif
        test(i == nRetry);
        com->deactivateObjectAdapter(adapters[1]);
        for(i = 0; i < nRetry && test->getAdapterName() == "Adapter33"; i++);
#if TARGET_OS_IPHONE > 0
        if(i != nRetry)
        {
            test->ice_getConnection()->close(Ice::ICE_SCOPED_ENUM(ConnectionClose, GracefullyWithWait));
            for(i = 0; i < nRetry && test->getAdapterName() == "Adapter33"; i++);
        }
#endif
        test(i == nRetry);
        com->deactivateObjectAdapter(adapters[2]);

        try
        {
            test->getAdapterName();
        }
        catch(const Ice::ConnectFailedException&)
        {
        }
#ifdef _WIN32
        catch(const Ice::ConnectTimeoutException&)
        {
        }
#endif
        Ice::EndpointSeq endpoints = test->ice_getEndpoints();

        adapters.clear();

        //
        // Now, re-activate the adapters with the same endpoints in the opposite
        // order.
        //
        adapters.push_back(com->createObjectAdapter("Adapter36", endpoints[2]->toString()));
        for(i = 0; i < nRetry && test->getAdapterName() == "Adapter36"; i++);
#if TARGET_OS_IPHONE > 0
        if(i != nRetry)
        {
            test->ice_getConnection()->close(Ice::ICE_SCOPED_ENUM(ConnectionClose, GracefullyWithWait));
            for(i = 0; i < nRetry && test->getAdapterName() == "Adapter36"; i++);
        }
#endif
        test(i == nRetry);
        test->ice_getConnection()->close(Ice::ICE_SCOPED_ENUM(ConnectionClose, GracefullyWithWait));
        adapters.push_back(com->createObjectAdapter("Adapter35", endpoints[1]->toString()));
        for(i = 0; i < nRetry && test->getAdapterName() == "Adapter35"; i++);
#if TARGET_OS_IPHONE > 0
        if(i != nRetry)
        {
            test->ice_getConnection()->close(Ice::ICE_SCOPED_ENUM(ConnectionClose, GracefullyWithWait));
            for(i = 0; i < nRetry && test->getAdapterName() == "Adapter35"; i++);
        }
#endif
        test(i == nRetry);
        test->ice_getConnection()->close(Ice::ICE_SCOPED_ENUM(ConnectionClose, GracefullyWithWait));
        adapters.push_back(com->createObjectAdapter("Adapter34", endpoints[0]->toString()));
        for(i = 0; i < nRetry && test->getAdapterName() == "Adapter34"; i++);
#if TARGET_OS_IPHONE > 0
        if(i != nRetry)
        {
            test->ice_getConnection()->close(Ice::ICE_SCOPED_ENUM(ConnectionClose, GracefullyWithWait));
            for(i = 0; i < nRetry && test->getAdapterName() == "Adapter34"; i++);
        }
#endif
        test(i == nRetry);

        deactivate(com, adapters);
    }
    cout << "ok" << endl;

    cout << "testing per request binding with single endpoint... " << flush;
    {
        RemoteObjectAdapterPrxPtr adapter = com->createObjectAdapter("Adapter41", "default");

        TestIntfPrxPtr test1 = ICE_UNCHECKED_CAST(TestIntfPrx, adapter->getTestIntf()->ice_connectionCached(false));
        TestIntfPrxPtr test2 = ICE_UNCHECKED_CAST(TestIntfPrx, adapter->getTestIntf()->ice_connectionCached(false));
        test(!test1->ice_isConnectionCached());
        test(!test2->ice_isConnectionCached());
        test(test1->ice_getConnection() && test2->ice_getConnection());
        test(test1->ice_getConnection() == test2->ice_getConnection());

        test1->ice_ping();

        com->deactivateObjectAdapter(adapter);

        TestIntfPrxPtr test3 = ICE_UNCHECKED_CAST(TestIntfPrx, test1);
        try
        {
            test(test3->ice_getConnection() == test1->ice_getConnection());
            test(false);
        }
        catch(const Ice::ConnectFailedException&)
        {
        }
#ifdef _WIN32
        catch(const Ice::ConnectTimeoutException&)
        {
        }
#endif
    }
    cout << "ok" << endl;

    cout << "testing per request binding with multiple endpoints... " << flush;
    {
        vector<RemoteObjectAdapterPrxPtr> adapters;
        adapters.push_back(com->createObjectAdapter("Adapter51", "default"));
        adapters.push_back(com->createObjectAdapter("Adapter52", "default"));
        adapters.push_back(com->createObjectAdapter("Adapter53", "default"));

        TestIntfPrxPtr test = ICE_UNCHECKED_CAST(TestIntfPrx, createTestIntfPrx(adapters)->ice_connectionCached(false));
        test(!test->ice_isConnectionCached());

        set<string> names;
        names.insert("Adapter51");
        names.insert("Adapter52");
        names.insert("Adapter53");
        while(!names.empty())
        {
            names.erase(test->getAdapterName());
        }

        com->deactivateObjectAdapter(adapters[0]);

        names.insert("Adapter52");
        names.insert("Adapter53");
        while(!names.empty())
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
        vector<RemoteObjectAdapterPrxPtr> adapters;
        adapters.push_back(com->createObjectAdapter("AdapterAMI51", "default"));
        adapters.push_back(com->createObjectAdapter("AdapterAMI52", "default"));
        adapters.push_back(com->createObjectAdapter("AdapterAMI53", "default"));

        TestIntfPrxPtr test = ICE_UNCHECKED_CAST(TestIntfPrx, createTestIntfPrx(adapters)->ice_connectionCached(false));
        test(!test->ice_isConnectionCached());

        set<string> names;
        names.insert("AdapterAMI51");
        names.insert("AdapterAMI52");
        names.insert("AdapterAMI53");
        while(!names.empty())
        {
            names.erase(getAdapterNameWithAMI(test));
        }

        com->deactivateObjectAdapter(adapters[0]);

        names.insert("AdapterAMI52");
        names.insert("AdapterAMI53");
        while(!names.empty())
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
        vector<RemoteObjectAdapterPrxPtr> adapters;
        adapters.push_back(com->createObjectAdapter("Adapter61", "default"));
        adapters.push_back(com->createObjectAdapter("Adapter62", "default"));
        adapters.push_back(com->createObjectAdapter("Adapter63", "default"));

        TestIntfPrxPtr test = createTestIntfPrx(adapters);
        test = ICE_UNCHECKED_CAST(TestIntfPrx, test->ice_endpointSelection(Ice::ICE_ENUM(EndpointSelectionType, Ordered)));
        test(test->ice_getEndpointSelection() == Ice::ICE_ENUM(EndpointSelectionType, Ordered));
        test = ICE_UNCHECKED_CAST(TestIntfPrx, test->ice_connectionCached(false));
        test(!test->ice_isConnectionCached());
        const int nRetry = 5;
        int i;

        //
        // Ensure that endpoints are tried in order by deactiving the adapters
        // one after the other.
        //
        for(i = 0; i < nRetry && test->getAdapterName() == "Adapter61"; i++);
#if TARGET_OS_IPHONE > 0
        test(i >= nRetry - 1); // WORKAROUND: for connection establishment hang.
#else
        test(i == nRetry);
#endif
        com->deactivateObjectAdapter(adapters[0]);
        for(i = 0; i < nRetry && test->getAdapterName() == "Adapter62"; i++);
#if TARGET_OS_IPHONE > 0
        test(i >= nRetry - 1); // WORKAROUND: for connection establishment hang.
#else
        test(i == nRetry);
#endif
        com->deactivateObjectAdapter(adapters[1]);
        for(i = 0; i < nRetry && test->getAdapterName() == "Adapter63"; i++);
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
        catch(const Ice::ConnectFailedException&)
        {
        }
#ifdef _WIN32
        catch(const Ice::ConnectTimeoutException&)
        {
        }
#endif
        Ice::EndpointSeq endpoints = test->ice_getEndpoints();

        adapters.clear();

        //
        // Now, re-activate the adapters with the same endpoints in the opposite
        // order.
        //
        adapters.push_back(com->createObjectAdapter("Adapter66", endpoints[2]->toString()));
        for(i = 0; i < nRetry && test->getAdapterName() == "Adapter66"; i++);
#if TARGET_OS_IPHONE > 0
        test(i >= nRetry - 1); // WORKAROUND: for connection establishment hang.
#else
        test(i == nRetry);
#endif
        adapters.push_back(com->createObjectAdapter("Adapter65", endpoints[1]->toString()));
        for(i = 0; i < nRetry && test->getAdapterName() == "Adapter65"; i++);
#if TARGET_OS_IPHONE > 0
        test(i >= nRetry - 1); // WORKAROUND: for connection establishment hang.
#else
        test(i == nRetry);
#endif
        adapters.push_back(com->createObjectAdapter("Adapter64", endpoints[0]->toString()));
        for(i = 0; i < nRetry && test->getAdapterName() == "Adapter64"; i++);
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
        vector<RemoteObjectAdapterPrxPtr> adapters;
        adapters.push_back(com->createObjectAdapter("AdapterAMI61", "default"));
        adapters.push_back(com->createObjectAdapter("AdapterAMI62", "default"));
        adapters.push_back(com->createObjectAdapter("AdapterAMI63", "default"));

        TestIntfPrxPtr test = createTestIntfPrx(adapters);
        test = ICE_UNCHECKED_CAST(TestIntfPrx, test->ice_endpointSelection(Ice::ICE_ENUM(EndpointSelectionType, Ordered)));
        test(test->ice_getEndpointSelection() == Ice::ICE_ENUM(EndpointSelectionType, Ordered));
        test = ICE_UNCHECKED_CAST(TestIntfPrx, test->ice_connectionCached(false));
        test(!test->ice_isConnectionCached());
        const int nRetry = 5;
        int i;

        //
        // Ensure that endpoints are tried in order by deactiving the adapters
        // one after the other.
        //
        for(i = 0; i < nRetry && getAdapterNameWithAMI(test) == "AdapterAMI61"; i++);
#if TARGET_OS_IPHONE > 0
        test(i >= nRetry - 1); // WORKAROUND: for connection establishment hang.
#else
        test(i == nRetry);
#endif
        com->deactivateObjectAdapter(adapters[0]);
        for(i = 0; i < nRetry && getAdapterNameWithAMI(test) == "AdapterAMI62"; i++);
#if TARGET_OS_IPHONE > 0
        test(i >= nRetry - 1); // WORKAROUND: for connection establishment hang.
#else
        test(i == nRetry);
#endif
        com->deactivateObjectAdapter(adapters[1]);
        for(i = 0; i < nRetry && getAdapterNameWithAMI(test) == "AdapterAMI63"; i++);
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
        catch(const Ice::ConnectFailedException&)
        {
        }
#ifdef _WIN32
        catch(const Ice::ConnectTimeoutException&)
        {
        }
#endif
        Ice::EndpointSeq endpoints = test->ice_getEndpoints();

        adapters.clear();

        //
        // Now, re-activate the adapters with the same endpoints in the opposite
        // order.
        //
        adapters.push_back(com->createObjectAdapter("AdapterAMI66", endpoints[2]->toString()));
        for(i = 0; i < nRetry && getAdapterNameWithAMI(test) == "AdapterAMI66"; i++);
#if TARGET_OS_IPHONE > 0
        test(i >= nRetry - 1); // WORKAROUND: for connection establishment hang.
#else
        test(i == nRetry);
#endif
        adapters.push_back(com->createObjectAdapter("AdapterAMI65", endpoints[1]->toString()));
        for(i = 0; i < nRetry && getAdapterNameWithAMI(test) == "AdapterAMI65"; i++);
        test(i == nRetry);
        adapters.push_back(com->createObjectAdapter("AdapterAMI64", endpoints[0]->toString()));
        for(i = 0; i < nRetry && getAdapterNameWithAMI(test) == "AdapterAMI64"; i++);
        test(i == nRetry);

        deactivate(com, adapters);
    }
    cout << "ok" << endl;

    cout << "testing endpoint mode filtering... " << flush;
    {
        vector<RemoteObjectAdapterPrxPtr> adapters;
        adapters.push_back(com->createObjectAdapter("Adapter71", "default"));
        adapters.push_back(com->createObjectAdapter("Adapter72", "udp"));

        TestIntfPrxPtr test = createTestIntfPrx(adapters);
        test(test->getAdapterName() == "Adapter71");

        TestIntfPrxPtr testUDP = ICE_UNCHECKED_CAST(TestIntfPrx, test->ice_datagram());
        test(test->ice_getConnection() != testUDP->ice_getConnection());
        try
        {
            testUDP->getAdapterName();
        }
        catch(const Ice::TwowayOnlyException&)
        {
        }
        catch(const IceUtil::IllegalArgumentException&)
        {
        }
    }
    cout << "ok" << endl;

    if(!communicator->getProperties()->getProperty("Ice.Plugin.IceSSL").empty() &&
       communicator->getProperties()->getProperty("Ice.Default.Protocol") == "ssl")
    {
        cout << "testing unsecure vs. secure endpoints... " << flush;
        {
            vector<RemoteObjectAdapterPrxPtr> adapters;
            adapters.push_back(com->createObjectAdapter("Adapter81", "ssl"));
            adapters.push_back(com->createObjectAdapter("Adapter82", "tcp"));

            TestIntfPrxPtr test = createTestIntfPrx(adapters);
            int i;
            for(i = 0; i < 5; i++)
            {
                test(test->getAdapterName() == "Adapter82");
                test->ice_getConnection()->close(Ice::ICE_SCOPED_ENUM(ConnectionClose, GracefullyWithWait));
            }

            TestIntfPrxPtr testSecure = ICE_UNCHECKED_CAST(TestIntfPrx, test->ice_secure(true));
            test(testSecure->ice_isSecure());
            testSecure = ICE_UNCHECKED_CAST(TestIntfPrx, test->ice_secure(false));
            test(!testSecure->ice_isSecure());
            testSecure = ICE_UNCHECKED_CAST(TestIntfPrx, test->ice_secure(true));
            test(testSecure->ice_isSecure());
            test(test->ice_getConnection() != testSecure->ice_getConnection());

            com->deactivateObjectAdapter(adapters[1]);

            for(i = 0; i < 5; i++)
            {
                test(test->getAdapterName() == "Adapter81");
                test->ice_getConnection()->close(Ice::ICE_SCOPED_ENUM(ConnectionClose, GracefullyWithWait));
            }

            com->createObjectAdapter("Adapter83", (test->ice_getEndpoints()[1])->toString()); // Reactive tcp OA.

            for(i = 0; i < 5; i++)
            {
                test(test->getAdapterName() == "Adapter83");
                test->ice_getConnection()->close(Ice::ICE_SCOPED_ENUM(ConnectionClose, GracefullyWithWait));
            }

            com->deactivateObjectAdapter(adapters[0]);
            try
            {
                testSecure->ice_ping();
                test(false);
            }
            catch(const Ice::ConnectFailedException&)
            {
            }
#ifdef _WIN32
            catch(const Ice::ConnectTimeoutException&)
            {
            }
#endif

            deactivate(com, adapters);
        }
        cout << "ok" << endl;
    }

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
            str << "tcp -p " << getTestPort(communicator->getProperties(), 2);
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
        anyboth->setProperty("Adapter.PublishedEndpoints", endpoint + " -p 12012:" + endpoint + " -p 12012");

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
        for(vector<Ice::PropertiesPtr>::const_iterator p = serverProps.begin(); p != serverProps.end(); ++p)
        {
            Ice::InitializationData serverInitData;
            serverInitData.properties = *p;
            Ice::CommunicatorPtr serverCommunicator = Ice::initialize(serverInitData);
            Ice::ObjectAdapterPtr oa;
            try
            {
                oa = serverCommunicator->createObjectAdapter("Adapter");
                oa->activate();
            }
            catch(const Ice::DNSException&)
            {
                serverCommunicator->destroy();
                continue; // IP version not supported.
            }
            catch(const Ice::SocketException&)
            {
                if(*p == ipv6)
                {
                    ipv6NotSupported = true;
                }
                serverCommunicator->destroy();
                continue; // IP version not supported.
            }

            // Ensure the published endpoints are actually valid. On
            // Fedora, binding to "localhost" with IPv6 only works but
            // resolving localhost don't return the IPv6 adress.
            Ice::ObjectPrxPtr prx = oa->createProxy(Ice::stringToIdentity("dummy"));
            try
            {
                prx->ice_collocationOptimized(false)->ice_ping();
            }
            catch(const Ice::LocalException&)
            {
                serverCommunicator->destroy();
                continue; // IP version not supported.
            }

            string strPrx = prx->ice_toString();
            for(vector<Ice::PropertiesPtr>::const_iterator q = clientProps.begin(); q != clientProps.end(); ++q)
            {
                Ice::InitializationData clientInitData;
                clientInitData.properties = *q;
                Ice::CommunicatorHolder clientCommunicator(clientInitData);
                Ice::ObjectPrxPtr prx = clientCommunicator->stringToProxy(strPrx);
                try
                {
                    prx->ice_ping();
                    test(false);
                }
                catch(const Ice::ObjectNotExistException&)
                {
                    // Expected, no object registered.
                }
                catch(const Ice::DNSException&)
                {
                    // Expected if no IPv4 or IPv6 address is
                    // associated to localhost or if trying to connect
                    // to an any endpoint with the wrong IP version,
                    // e.g.: resolving an IPv4 address when only IPv6
                    // is enabled fails with a DNS exception.
                }
                catch(const Ice::SocketException&)
                {
                    test((*p == ipv4 && *q == ipv6) || (*p == ipv6 && *q == ipv4) ||
                         (*p == bothPreferIPv4 && *q == ipv6) || (*p == bothPreferIPv6 && *q == ipv4) ||
                         (*p == bothPreferIPv6 && *q == ipv6 && ipv6NotSupported) ||
                         (*p == anyipv4 && *q == ipv6) || (*p == anyipv6 && *q == ipv4) ||
                         (*p == localipv4 && *q == ipv6) || (*p == localipv6 && *q == ipv4) ||
                         (*p == ipv6 && *q == bothPreferIPv4) || (*p == ipv6 && *q == bothPreferIPv6) ||
                         (*p == bothPreferIPv6 && *q == ipv6));
                }
            }
            serverCommunicator->destroy();
        }

        cout << "ok" << endl;
    }

    //
    // On Windows, the FD limit is very high and there's no way to limit the number of FDs
    // for the server so we don't run this test.
    //
#if !defined(_WIN32) && (!defined(__APPLE__) || TARGET_OS_IPHONE == 0)
    {
        cout << "testing FD limit... " << flush;

        RemoteObjectAdapterPrxPtr adapter = com->createObjectAdapter("Adapter", "default");

        TestIntfPrxPtr test = adapter->getTestIntf();
        int i = 0;
        while(true)
        {
            try
            {
                ostringstream os;
                os << i;
                test->ice_connectionId(os.str())->ice_ping();
                ++i;
            }
            catch(const Ice::LocalException&)
            {
                break;
            }
        }

        try
        {
            ostringstream os;
            os << i;
            test->ice_connectionId(os.str())->ice_ping();
            test(false);
        }
        catch(const Ice::ConnectionRefusedException&)
        {
            // Close the connection now to free a FD (it could be done after the sleep but
            // there could be race condiutation since the connection might not be closed
            // immediately due to threading).
            test->ice_connectionId("0")->ice_getConnection()->close(
                Ice::ICE_SCOPED_ENUM(ConnectionClose, GracefullyWithWait));

            //
            // The server closed the acceptor, wait one second and retry after freeing a FD.
            //
            IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(1100));
            try
            {
                ostringstream os;
                os << i;
                test->ice_connectionId(os.str())->ice_ping();
            }
            catch(const Ice::LocalException&)
            {
                test(false);
            }
        }
        catch(const Ice::LocalException&)
        {
            // The server didn't close the acceptor but we still get a failure (it's possible
            // that the client reached the FD limit depending on the server we are running
            // against...).
        }

        cout << "ok" << endl;
    }
#endif

    com->shutdown();
}
