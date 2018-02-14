// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestCommon.h>
#include <Test.h>

using namespace std;
using namespace Test;
namespace
{

string
toString(int value)
{
    ostringstream os;
    os << value;
    return os.str();
}

class LoggerI : public Ice::Logger, private IceUtil::Mutex
{
public:

    LoggerI() : _started(false)
    {
    }

    void
    start()
    {
        Lock sync(*this);
        _started = true;
        dump();
    }

    virtual void
    print(const std::string& msg)
    {
        Lock sync(*this);
        _messages.push_back(msg);
        if(_started)
        {
            dump();
        }
    }

    virtual void
    trace(const std::string& category, const std::string& message)
    {
        Lock sync(*this);
        _messages.push_back("[" + category + "] " + message);
        if(_started)
        {
            dump();
        }
    }

    virtual void
    warning(const std::string& message)
    {
        Lock sync(*this);
        _messages.push_back("warning: " + message);
        if(_started)
        {
            dump();
        }
    }

    virtual void
    error(const std::string& message)
    {
        Lock sync(*this);
        _messages.push_back("error: " + message);
        if(_started)
        {
            dump();
        }
    }

    virtual string
    getPrefix()
    {
        return "";
    }

    virtual Ice::LoggerPtr
    cloneWithPrefix(const std::string&)
    {
        return this;
    }

private:

    void
    dump()
    {
        for(vector<string>::const_iterator p = _messages.begin(); p != _messages.end(); ++p)
        {
            cout << *p << endl;
        }
        _messages.clear();
    }

    bool _started;
    vector<string> _messages;
};
typedef IceUtil::Handle<LoggerI> LoggerIPtr;

class TestCase : public IceUtil::Thread, protected Ice::ConnectionCallback, protected IceUtil::Monitor<IceUtil::Mutex>
{
public:

    TestCase(const string& name, const RemoteCommunicatorPrx& com) :
        _name(name), _com(com), _logger(new LoggerI()),
        _clientACMTimeout(-1), _clientACMClose(-1), _clientACMHeartbeat(-1),
        _serverACMTimeout(-1), _serverACMClose(-1), _serverACMHeartbeat(-1),
        _heartbeat(0), _closed(false)
    {
    }

    void
    init()
    {
        _adapter = _com->createObjectAdapter(_serverACMTimeout, _serverACMClose, _serverACMHeartbeat);

        Ice::InitializationData initData;
        initData.properties = _com->ice_getCommunicator()->getProperties()->clone();
        initData.logger = _logger;
        initData.properties->setProperty("Ice.ACM.Timeout", "1");
        if(_clientACMTimeout >= 0)
        {
            initData.properties->setProperty("Ice.ACM.Client.Timeout", toString(_clientACMTimeout));
        }
        if(_clientACMClose >= 0)
        {
            initData.properties->setProperty("Ice.ACM.Client.Close", toString(_clientACMClose));
        }
        if(_clientACMHeartbeat >= 0)
        {
            initData.properties->setProperty("Ice.ACM.Client.Heartbeat", toString(_clientACMHeartbeat));
        }
        //initData.properties->setProperty("Ice.Trace.Protocol", "2");
        //initData.properties->setProperty("Ice.Trace.Network", "2");
        _communicator = Ice::initialize(initData);
    }

    void
    destroy()
    {
        _adapter->deactivate();
        _communicator->destroy();
    }

    void
    join()
    {
        cout << "testing " << _name << "... " << flush;
        _logger->start();
        getThreadControl().join();
        if(_msg.empty())
        {
            cout << "ok" << endl;
        }
        else
        {
            cout << "failed! " << endl << _msg;
            test(false);
        }
    }

    virtual void
    run()
    {
        TestIntfPrx proxy = TestIntfPrx::uncheckedCast(_communicator->stringToProxy(
                                                           _adapter->getTestIntf()->ice_toString()));
        try
        {
            proxy->ice_getConnection()->setCallback(this);
            runTestCase(_adapter, proxy);
        }
        catch(const std::exception& ex)
        {
            _msg = string("unexpected exception:\n") + ex.what();
        }
        catch(...)
        {
            _msg = "unknown exception";
        }
    }

    virtual void
    heartbeat(const Ice::ConnectionPtr&)
    {
        Lock sync(*this);
        ++_heartbeat;
    }

    virtual void
    closed(const Ice::ConnectionPtr&)
    {
        Lock sync(*this);
        _closed = true;
        notify();
    }

    void
    waitForClosed()
    {
        Lock sync(*this);
        IceUtil::Time now = IceUtil::Time::now(IceUtil::Time::Monotonic);
        while(!_closed)
        {
            timedWait(IceUtil::Time::seconds(1));
            if(IceUtil::Time::now(IceUtil::Time::Monotonic) - now > IceUtil::Time::seconds(1))
            {
                test(false); // Waited for more than 1s for close, something's wrong.
            }
        }
    }

    virtual void runTestCase(const RemoteObjectAdapterPrx&, const TestIntfPrx&) = 0;

    void
    setClientACM(int timeout, int close, int heartbeat)
    {
        _clientACMTimeout = timeout;
        _clientACMClose = close;
        _clientACMHeartbeat = heartbeat;
    }

    void
    setServerACM(int timeout, int close, int heartbeat)
    {
        _serverACMTimeout = timeout;
        _serverACMClose = close;
        _serverACMHeartbeat = heartbeat;
    }

protected:

    const string _name;
    const RemoteCommunicatorPrx _com;
    string _msg;
    LoggerIPtr _logger;

    Ice::CommunicatorPtr _communicator;
    RemoteObjectAdapterPrx _adapter;

    int _clientACMTimeout;
    int _clientACMClose;
    int _clientACMHeartbeat;
    int _serverACMTimeout;
    int _serverACMClose;
    int _serverACMHeartbeat;

    int _heartbeat;
    bool _closed;
};
typedef IceUtil::Handle<TestCase> TestCasePtr;

}

void
allTests(const Ice::CommunicatorPtr& communicator)
{
    string ref = "communicator:default -p 12010";
    RemoteCommunicatorPrx com = RemoteCommunicatorPrx::uncheckedCast(communicator->stringToProxy(ref));

    vector<TestCasePtr> tests;

    class InvocationHeartbeatTest : public TestCase
    {
    public:

        InvocationHeartbeatTest(const RemoteCommunicatorPrx& com) :
            TestCase("invocation heartbeat", com)
        {
        }

        virtual void runTestCase(const RemoteObjectAdapterPrx& adapter, const TestIntfPrx& proxy)
        {
            proxy->sleep(2);

            Lock sync(*this);
            test(_heartbeat >= 2);
        }
    };

    class InvocationHeartbeatOnHoldTest : public TestCase
    {
    public:

        InvocationHeartbeatOnHoldTest(const RemoteCommunicatorPrx& com) :
            TestCase("invocation with heartbeat on hold", com)
        {
            // Use default ACM configuration.
        }

        virtual void runTestCase(const RemoteObjectAdapterPrx& adapter, const TestIntfPrx& proxy)
        {
            try
            {
                // When the OA is put on hold, connections shouldn't
                // send heartbeats, the invocation should therefore
                // fail.
                proxy->sleepAndHold(10);
                test(false);
            }
            catch(const Ice::ConnectionTimeoutException&)
            {
                adapter->activate();
                proxy->interruptSleep();

                waitForClosed();
            }
        }
    };

    class InvocationNoHeartbeatTest : public TestCase
    {
    public:

        InvocationNoHeartbeatTest(const RemoteCommunicatorPrx& com) :
            TestCase("invocation with no heartbeat", com)
        {
            setServerACM(1, 2, 0); // Disable heartbeat on invocations
        }

        virtual void runTestCase(const RemoteObjectAdapterPrx& adapter, const TestIntfPrx& proxy)
        {
            try
            {
                // Heartbeats are disabled on the server, the
                // invocation should fail since heartbeats are
                // expected.
                proxy->sleep(10);
                test(false);
            }
            catch(const Ice::ConnectionTimeoutException&)
            {
                proxy->interruptSleep();

                waitForClosed();

                Lock sync(*this);
                test(_heartbeat == 0);
            }
        }
    };

    class InvocationHeartbeatCloseOnIdleTest : public TestCase
    {
    public:

        InvocationHeartbeatCloseOnIdleTest(const RemoteCommunicatorPrx& com) :
            TestCase("invocation with no heartbeat and close on idle", com)
        {
            setClientACM(1, 1, 0); // Only close on idle.
            setServerACM(1, 2, 0); // Disable heartbeat on invocations
        }

        virtual void runTestCase(const RemoteObjectAdapterPrx& adapter, const TestIntfPrx& proxy)
        {
            // No close on invocation, the call should succeed this
            // time.
            proxy->sleep(2);

            Lock sync(*this);
            test(_heartbeat == 0);
            test(!_closed);
        }
    };

    class CloseOnIdleTest : public TestCase
    {
    public:

        CloseOnIdleTest(const RemoteCommunicatorPrx& com) : TestCase("close on idle", com)
        {
            setClientACM(1, 1, 0); // Only close on idle
        }

        virtual void runTestCase(const RemoteObjectAdapterPrx& adapter, const TestIntfPrx& proxy)
        {
            IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(1500)); // Idle for 1.5 seconds

            waitForClosed();

            Lock sync(*this);
            test(_heartbeat == 0);
        }
    };

    class CloseOnInvocationTest : public TestCase
    {
    public:

        CloseOnInvocationTest(const RemoteCommunicatorPrx& com) : TestCase("close on invocation", com)
        {
            setClientACM(1, 2, 0); // Only close on invocation
        }

        virtual void runTestCase(const RemoteObjectAdapterPrx& adapter, const TestIntfPrx& proxy)
        {
            IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(1500)); // Idle for 1.5 seconds

            Lock sync(*this);
            test(_heartbeat == 0);
            test(!_closed);
        }
    };

    class CloseOnIdleAndInvocationTest : public TestCase
    {
    public:

        CloseOnIdleAndInvocationTest(const RemoteCommunicatorPrx& com) : TestCase("close on idle and invocation", com)
        {
            setClientACM(1, 3, 0); // Only close on idle and invocation
        }

        virtual void runTestCase(const RemoteObjectAdapterPrx& adapter, const TestIntfPrx& proxy)
        {
            //
            // Put the adapter on hold. The server will not respond to
            // the graceful close. This allows to test whether or not
            // the close is graceful or forceful.
            //
            adapter->hold();
            IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(1500)); // Idle for 1.5 seconds

            {
                Lock sync(*this);
                test(_heartbeat == 0);
                test(!_closed); // Not closed yet because of graceful close.
            }

            adapter->activate();
            IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(500));

            waitForClosed();
        }
    };

    class ForcefulCloseOnIdleAndInvocationTest : public TestCase
    {
    public:

        ForcefulCloseOnIdleAndInvocationTest(const RemoteCommunicatorPrx& com) :
            TestCase("forceful close on idle and invocation", com)
        {
            setClientACM(1, 4, 0); // Only close on idle and invocation
        }

        virtual void runTestCase(const RemoteObjectAdapterPrx& adapter, const TestIntfPrx& proxy)
        {
            adapter->hold();
            IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(1500)); // Idle for 1.5 seconds

            waitForClosed();

            Lock sync(*this);
            test(_heartbeat == 0);
        }
    };

    class HeartbeatOnIdleTest : public TestCase
    {
    public:

        HeartbeatOnIdleTest(const RemoteCommunicatorPrx& com) : TestCase("heartbeat on idle", com)
        {
            setServerACM(1, -1, 2); // Enable server heartbeats.
        }

        virtual void runTestCase(const RemoteObjectAdapterPrx& adapter, const TestIntfPrx& proxy)
        {
            IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(2000));

            Lock sync(*this);
            test(_heartbeat >= 3);
        }
    };

    class HeartbeatAlwaysTest : public TestCase
    {
    public:

        HeartbeatAlwaysTest(const RemoteCommunicatorPrx& com) : TestCase("heartbeat always", com)
        {
            setServerACM(1, -1, 3); // Enable server heartbeats.
        }

        virtual void runTestCase(const RemoteObjectAdapterPrx& adapter, const TestIntfPrx& proxy)
        {
            for(int i = 0; i < 12; ++i)
            {
                proxy->ice_ping();
                IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(200));
            }

            Lock sync(*this);
            test(_heartbeat >= 3);
        }
    };

    class SetACMTest : public TestCase
    {
    public:

        SetACMTest(const RemoteCommunicatorPrx& com) : TestCase("setACM/getACM", com)
        {
            setClientACM(15, 4, 0);
        }

        virtual void runTestCase(const RemoteObjectAdapterPrx& adapter, const TestIntfPrx& proxy)
        {
            Ice::ACM acm;
            acm = proxy->ice_getCachedConnection()->getACM();
            test(acm.timeout == 15);
            test(acm.close == Ice::CloseOnIdleForceful);
            test(acm.heartbeat == Ice::HeartbeatOff);

            proxy->ice_getCachedConnection()->setACM(IceUtil::None, IceUtil::None, IceUtil::None);
            acm = proxy->ice_getCachedConnection()->getACM();
            test(acm.timeout == 15);
            test(acm.close == Ice::CloseOnIdleForceful);
            test(acm.heartbeat == Ice::HeartbeatOff);

            proxy->ice_getCachedConnection()->setACM(1, Ice::CloseOnInvocationAndIdle, Ice::HeartbeatAlways);
            acm = proxy->ice_getCachedConnection()->getACM();
            test(acm.timeout == 1);
            test(acm.close == Ice::CloseOnInvocationAndIdle);
            test(acm.heartbeat == Ice::HeartbeatAlways);

            // Make sure the client sends few heartbeats to the server
            proxy->waitForHeartbeat(2);
        }
    };

    tests.push_back(new InvocationHeartbeatTest(com));
    tests.push_back(new InvocationHeartbeatOnHoldTest(com));
    tests.push_back(new InvocationNoHeartbeatTest(com));
    tests.push_back(new InvocationHeartbeatCloseOnIdleTest(com));

    tests.push_back(new CloseOnIdleTest(com));
    tests.push_back(new CloseOnInvocationTest(com));
    tests.push_back(new CloseOnIdleAndInvocationTest(com));
    tests.push_back(new ForcefulCloseOnIdleAndInvocationTest(com));

    tests.push_back(new HeartbeatOnIdleTest(com));
    tests.push_back(new HeartbeatAlwaysTest(com));
    tests.push_back(new SetACMTest(com));

    for(vector<TestCasePtr>::const_iterator p = tests.begin(); p != tests.end(); ++p)
    {
        (*p)->init();
    }
    for(vector<TestCasePtr>::const_iterator p = tests.begin(); p != tests.end(); ++p)
    {
        (*p)->start();
    }
    for(vector<TestCasePtr>::const_iterator p = tests.begin(); p != tests.end(); ++p)
    {
        (*p)->join();
    }
    for(vector<TestCasePtr>::const_iterator p = tests.begin(); p != tests.end(); ++p)
    {
        (*p)->destroy();
    }

    cout << "shutting down... " << flush;
    com->shutdown();
    cout << "ok" << endl;
}
