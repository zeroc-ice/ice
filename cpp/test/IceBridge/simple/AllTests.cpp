//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <TestHelper.h>
#include <Test.h>
#include <mutex>
#include <chrono>
#include <atomic>

using namespace std;
using namespace std::chrono_literals;

namespace
{

class CallbackI final : public Test::Callback
{
public:

    void
    ping(const Ice::Current&) override
    {
        ++_count;
    }

    int
    getCount(const Ice::Current&) override
    {
        return _count;
    }

    void
    datagram(const Ice::Current& c) override
    {
        test(c.con->getEndpoint()->getInfo()->datagram());
        ++_datagramCount;
    }

    int
    getDatagramCount(const Ice::Current&) override
    {
        return _datagramCount;
    }

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
    auto prx = communicator->stringToProxy("test:" + helper->getTestEndpoint(1) + ":" +
                                           helper->getTestEndpoint(1, "udp"));
    test(prx);
    auto cl = Ice::checkedCast<Test::MyClassPrx>(prx);
    cl->ice_ping();
    cout << "ok" << endl;

    cout << "testing datagrams... " << flush;
    {
        for(int i = 0; i < 20; i++)
        {
            cl->ice_datagram()->datagram();
        }
        int nRetry = 20;
        while(cl->getDatagramCount() < 10 && --nRetry > 0)
        {
            this_thread::sleep_for(50ms);
        }
        test(cl->getDatagramCount() >= 10);
    }
    cout << "ok" << endl;

    cout << "testing connection close... " << flush;
    {
        auto clc =
            Ice::checkedCast<Test::MyClassPrx>(cl->ice_getConnection()->createProxy(cl->ice_getIdentity()));
        clc->ice_ping();
        clc->closeConnection(false);
        int nRetry = 20;
        while(--nRetry > 0)
        {
            try
            {
                clc->ice_ping();
                test(false);
            }
            catch(const Ice::CloseConnectionException&)
            {
                // Wait for the CloseConnectionException before continuing
                break;
            }
            catch(const Ice::UnknownLocalException& ex)
            {
                // The bridge forwards the CloseConnectionException from the server as an
                // UnknownLocalException. It eventually closes the connection when notified
                // of the connection close.
                test(ex.unknown.find("CloseConnectionException") != string::npos);
            }
            this_thread::sleep_for(1ms);
        }
        try
        {
            clc->ice_ping();
            test(false);
        }
        catch(const Ice::CloseConnectionException&)
        {
        }
    }
    cout << "ok" << endl;

    cout << "testing multiple connections... " << flush;
    {
        test(cl->getConnectionInfo() == cl->getConnectionInfo());
        int nRetry = 20;
        while(cl->getConnectionCount() != 2 && --nRetry > 0)
        {
            this_thread::sleep_for(50ms);
        }
        test(cl->getConnectionCount() == 2);
        test(cl->ice_connectionId("other")->getConnectionInfo() != cl->getConnectionInfo());
        test(cl->getConnectionCount() == 3);
        cl->ice_connectionId("other")->ice_getConnection()->close(Ice::ConnectionClose::Gracefully);
        nRetry = 20;
        while(cl->getConnectionCount() != 2 && --nRetry > 0)
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
        for(int i = 0; i < 10; ++i)
        {
            ostringstream os;
            os << i;
            auto p = cl->ice_connectionId(os.str());
            for(int j = 0; j < 20; ++j)
            {
                p->incCounterAsync(++counter, nullptr);
            }
            cl->waitCounter(counter);
            p->closeConnection(false);
        }
        for(int i = 0; i < 10; ++i)
        {
            ostringstream os;
            os << i;
            auto p = cl->ice_connectionId(os.str())->ice_oneway();
            for(int j = 0; j < 20; ++j)
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
        for(int i = 0; i < 20; i++)
        {
            p->callDatagramCallback();
        }
        int nRetry = 20;
        while(cl->getCallbackDatagramCount() < 10 && --nRetry > 0)
        {
            this_thread::sleep_for(50ms);
        }
        test(cl->getCallbackDatagramCount() >= 10);
    }
    cout << "ok" << endl;

    cout << "testing router... " << flush;
    {
        auto base = communicator->stringToProxy("Ice/RouterFinder:" + helper->getTestEndpoint(1));
        auto finder = Ice::checkedCast<Ice::RouterFinderPrx>(base);
        auto router = finder->getRouter();
        base = communicator->stringToProxy("test")->ice_router(router);
        auto p = Ice::checkedCast<Test::MyClassPrx>(base);
        p->ice_ping();
    }
    cout << "ok" << endl;

    cout << "testing heartbeats... " << flush;
    {
        test(cl->getHeartbeatCount() == 0); // No heartbeats enabled by default

        auto p = cl->ice_connectionId("heartbeat");
        p->ice_getConnection()->setACM(1, Ice::nullopt, Ice::ACMHeartbeat::HeartbeatAlways);

        auto p2 = cl->ice_connectionId("heartbeat2");
        atomic_int counter = 0;
        p2->ice_getConnection()->setHeartbeatCallback([&counter](const auto&)
                                                      {
                                                          counter++;
                                                      });
        p2->enableHeartbeats();

        int nRetry = 20;
        while((p->getHeartbeatCount() < 1 || counter.load() < 1) && --nRetry > 0)
        {
            this_thread::sleep_for(500ms); // TODO: check sleep time
        }
        test(p->getHeartbeatCount() > 0 && counter.load() > 0);
    }
    cout << "ok" << endl;

    cout << "testing server shutdown... " << flush;
    cl->shutdown();
    cout << "ok" << endl;

    cout << "testing bridge shutdown... " << flush;
    auto admin = communicator->stringToProxy("IceBridge/admin:" + helper->getTestEndpoint(2, "tcp"));
    auto process = Ice::checkedCast<Ice::ProcessPrx>(admin->ice_facet("Process"));
    process->shutdown();
    cout << "ok" << endl;
}
