//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <TestHelper.h>
#include <Test.h>

using namespace std;

namespace
{

class CallbackI : public Test::Callback
{
public:

    CallbackI() : _count(0), _datagramCount(0)
    {
    }

    virtual void
    ping(const Ice::Current&)
    {
        ++_count;
    }

    virtual int
    getCount(const Ice::Current&)
    {
        return _count;
    }

    virtual void
    datagram(const Ice::Current& c)
    {
        test(c.con->getEndpoint()->getInfo()->datagram());
        ++_datagramCount;
    }

    virtual int
    getDatagramCount(const Ice::Current&)
    {
        return _datagramCount;
    }

private:

    int _count;
    int _datagramCount;
};

class HeartbeatCallbackI : public Ice::HeartbeatCallback, private IceUtil::Mutex
{
public:

    HeartbeatCallbackI() : _count(0)
    {
    }

    virtual void heartbeat(const Ice::ConnectionPtr&)
    {
        Lock sync(*this);
        ++_count;
    }

    int getCount() const
    {
        Lock sync(*this);
        return _count;
    }

private:

    int _count;
};

}

void
allTests(Test::TestHelper* helper)
{
    Ice::CommunicatorPtr communicator = helper->communicator();
    cout << "testing connection to bridge... " << flush;
    Ice::ObjectPrx prx = communicator->stringToProxy("test:" + helper->getTestEndpoint(1) + ":" +
                                                      helper->getTestEndpoint(1, "udp"));
    test(prx);
    Test::MyClassPrx cl = Ice::checkedCast<Test::MyClassPrx>(prx);
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
            IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(50));
        }
        test(cl->getDatagramCount() >= 10);
    }
    cout << "ok" << endl;

    cout << "testing connection close... " << flush;
    {
        Test::MyClassPrx clc =
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
            IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(1));
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
            IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(50));
        }
        test(cl->getConnectionCount() == 2);
        test(cl->ice_connectionId("other")->getConnectionInfo() != cl->getConnectionInfo());
        test(cl->getConnectionCount() == 3);
        cl->ice_connectionId("other")->ice_getConnection()->close(Ice::ConnectionCloseGracefully);
        nRetry = 20;
        while(cl->getConnectionCount() != 2 && --nRetry > 0)
        {
            IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(50));
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
            Test::MyClassPrx p = cl->ice_connectionId(os.str());
            for(int j = 0; j < 20; ++j)
            {
                p->begin_incCounter(++counter);
            }
            cl->waitCounter(counter);
            p->closeConnection(false);
        }
        for(int i = 0; i < 10; ++i)
        {
            ostringstream os;
            os << i;
            Test::MyClassPrx p = cl->ice_connectionId(os.str())->ice_oneway();
            for(int j = 0; j < 20; ++j)
            {
                p->begin_incCounter(++counter);
            }
            cl->waitCounter(counter);
            p->closeConnection(false);
        }
    }
    cout << "ok" << endl;

    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("");
    Ice::Identity id;
    id.name = "callback";
    adapter->add(new CallbackI(), id);

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
        Test::MyClassPrx p = cl->ice_datagram();
        p->ice_getConnection()->setAdapter(adapter);
        for(int i = 0; i < 20; i++)
        {
            p->callDatagramCallback();
        }
        int nRetry = 20;
        while(cl->getCallbackDatagramCount() < 10 && --nRetry > 0)
        {
            IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(50));
        }
        test(cl->getCallbackDatagramCount() >= 10);
    }
    cout << "ok" << endl;

    cout << "testing router... " << flush;
    {
        Ice::ObjectPrx base = communicator->stringToProxy("Ice/RouterFinder:" + helper->getTestEndpoint(1));
        Ice::RouterFinderPrx finder = Ice::checkedCast<Ice::RouterFinderPrx>(base);
        Ice::RouterPrx router = finder->getRouter();
        base = communicator->stringToProxy("test")->ice_router(router);
        Test::MyClassPrx p = Ice::checkedCast<Test::MyClassPrx>(base);
        p->ice_ping();
    }
    cout << "ok" << endl;

    cout << "testing heartbeats... " << flush;
    {
        test(cl->getHeartbeatCount() == 0); // No heartbeats enabled by default

        Test::MyClassPrx p = cl->ice_connectionId("heartbeat");
        p->ice_getConnection()->setACM(1, IceUtil::None, Ice::HeartbeatAlways);

        Test::MyClassPrx p2 = cl->ice_connectionId("heartbeat2");
        HeartbeatCallbackI* heartbeat = new HeartbeatCallbackI();
        p2->ice_getConnection()->setHeartbeatCallback(heartbeat);
        p2->enableHeartbeats();

        int nRetry = 20;
        while((p->getHeartbeatCount() < 1 || heartbeat->getCount() < 1) && --nRetry > 0)
        {
            IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(500));
        }
        test(p->getHeartbeatCount() > 0 && heartbeat->getCount() > 0);
    }
    cout << "ok" << endl;

    cout << "testing server shutdown... " << flush;
    cl->shutdown();
    cout << "ok" << endl;

    cout << "testing bridge shutdown... " << flush;
    Ice::ObjectPrx admin = communicator->stringToProxy("IceBridge/admin:" + helper->getTestEndpoint(2, "tcp"));
    Ice::ProcessPrx process = Ice::checkedCast<Ice::ProcessPrx>(admin->ice_facet("Process"));
    process->shutdown();
    cout << "ok" << endl;
}
