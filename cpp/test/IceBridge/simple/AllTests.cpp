// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "Test.h"
#include "TestHelper.h"
#include <chrono>
#include <thread>

using namespace std;
using namespace std::chrono_literals;

namespace
{
    class CallbackI final : public Test::Callback
    {
    public:
        void ping(const Ice::Current&) override { ++_count; }

        int getCount(const Ice::Current&) override { return _count; }

        void datagram(const Ice::Current& c) override
        {
            test(c.con->getEndpoint()->getInfo()->datagram());
            ++_datagramCount;
        }

        int getDatagramCount(const Ice::Current&) override { return _datagramCount; }

    private:
        int _count = 0;
        int _datagramCount = 0;
    };
}

void
allTests(Test::TestHelper* helper)
{
    auto communicator = helper->communicator();
    cout << "testing connection to bridge... " << flush;

    Test::MyClassPrx cl(communicator, "test:" + helper->getTestEndpoint(1) + ":" + helper->getTestEndpoint(1, "udp"));
    cl->ice_ping();
    cout << "ok" << endl;

    cout << "testing datagrams... " << flush;
    {
        for (int i = 0; i < 20; i++)
        {
            cl->ice_datagram()->datagram();
        }
        int nRetry = 20;
        while (cl->getDatagramCount() < 10 && --nRetry > 0)
        {
            this_thread::sleep_for(50ms);
        }
        test(cl->getDatagramCount() >= 10);
    }
    cout << "ok" << endl;

    cout << "testing connection close... " << flush;
    {
        Test::MyClassPrx clc = cl->ice_fixed(cl->ice_getConnection());
        clc->ice_ping();
        clc->closeConnection(false);
        int nRetry = 20;
        while (--nRetry > 0)
        {
            try
            {
                clc->ice_ping();
                test(false);
            }
            catch (const Ice::CloseConnectionException&)
            {
                // Wait for the CloseConnectionException before continuing
                break;
            }
            catch (const Ice::UnknownLocalException& ex)
            {
                // The bridge forwards the CloseConnectionException from the server as an
                // UnknownLocalException. It eventually closes the connection when notified
                // of the connection close.
                string message{ex.what()};
                test(message.find("::Ice::CloseConnectionException") != string::npos);
            }
            this_thread::sleep_for(1ms);
        }
        try
        {
            clc->ice_ping();
            test(false);
        }
        catch (const Ice::CloseConnectionException&)
        {
        }
    }
    cout << "ok" << endl;

    cout << "testing multiple connections... " << flush;
    {
        test(cl->getConnectionInfo() == cl->getConnectionInfo());
        int nRetry = 20;
        while (cl->getConnectionCount() != 2 && --nRetry > 0)
        {
            this_thread::sleep_for(50ms);
        }
        test(cl->getConnectionCount() == 2);
        test(cl->ice_connectionId("other")->getConnectionInfo() != cl->getConnectionInfo());
        test(cl->getConnectionCount() == 3);
        cl->ice_connectionId("other")->ice_getConnection()->close(nullptr, nullptr);
        nRetry = 20;
        while (cl->getConnectionCount() != 2 && --nRetry > 0)
        {
            this_thread::sleep_for(50ms);
        }
        test(cl->getConnectionCount() == 2);
    }
    cout << "ok" << endl;

    cout << "testing ordering... " << flush;
    {
        //
        // Make sure ordering is preserved on connection establishemnt.
        //
        int counter = 0;
        for (int i = 0; i < 10; ++i)
        {
            ostringstream os;
            os << i;
            auto p = cl->ice_connectionId(os.str());
            for (int j = 0; j < 20; ++j)
            {
                p->incCounterAsync(++counter, nullptr);
            }
            cl->waitCounter(counter);
            p->closeConnection(false);
        }
        for (int i = 0; i < 10; ++i)
        {
            ostringstream os;
            os << i;
            auto p = cl->ice_connectionId(os.str())->ice_oneway();
            for (int j = 0; j < 20; ++j)
            {
                p->incCounterAsync(++counter, nullptr);
            }
            cl->waitCounter(counter);
            p->closeConnection(false);
        }
    }
    cout << "ok" << endl;

    auto adapter = communicator->createObjectAdapter("");
    Ice::Identity id;
    id.name = "callback";
    adapter->add(make_shared<CallbackI>(), id);

    cout << "testing bi-dir callbacks... " << flush;
    {
        cl->ice_getConnection()->setAdapter(adapter);
        cl->callCallback();
        cl->callCallback();
        cl->callCallback();
        test(cl->getCallbackCount() == 3);
        cl->callCallback();
        test(cl->getCallbackCount() == 4);
    }
    cout << "ok" << endl;

    cout << "testing datagram bi-dir callbacks... " << flush;
    {
        auto p = cl->ice_datagram();
        p->ice_getConnection()->setAdapter(adapter);
        for (int i = 0; i < 20; i++)
        {
            p->callDatagramCallback();
        }
        int nRetry = 20;
        while (cl->getCallbackDatagramCount() < 10 && --nRetry > 0)
        {
            this_thread::sleep_for(50ms);
        }
        test(cl->getCallbackDatagramCount() >= 10);
    }
    cout << "ok" << endl;

    cout << "testing router... " << flush;
    {
        Ice::RouterFinderPrx finder(communicator, "Ice/RouterFinder:" + helper->getTestEndpoint(1));
        auto router = finder->getRouter();
        auto p = Test::MyClassPrx(communicator, "test")->ice_router(router);
        p->ice_ping();
    }
    cout << "ok" << endl;

    cout << "testing server shutdown... " << flush;
    cl->shutdown();
    cout << "ok" << endl;

    cout << "testing bridge shutdown... " << flush;
    Ice::ObjectPrx admin(communicator, "IceBridge/admin:" + helper->getTestEndpoint(2, "tcp"));
    auto process = admin->ice_facet<Ice::ProcessPrx>("Process");
    process->shutdown();
    cout << "ok" << endl;
}
