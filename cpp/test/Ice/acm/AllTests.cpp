//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "Ice/Ice.h"
#include "Test.h"
#include "TestHelper.h"

#include <chrono>
#include <stdexcept>
#include <thread>

using namespace std;
using namespace Test;
namespace
{
    string toString(int value)
    {
        ostringstream os;
        os << value;
        return os.str();
    }

    class LoggerI final : public Ice::Logger, public enable_shared_from_this<LoggerI>
    {
    public:
        LoggerI() : _started(false) {}

        void start()
        {
            lock_guard lock(_mutex);
            _started = true;
            dump();
        }

        void print(const string& msg) final
        {
            lock_guard lock(_mutex);
            _messages.push_back(msg);
            if (_started)
            {
                dump();
            }
        }

        void trace(const string& category, const string& message) final
        {
            lock_guard lock(_mutex);
            _messages.push_back("[" + category + "] " + message);
            if (_started)
            {
                dump();
            }
        }

        void warning(const string& message) final
        {
            lock_guard lock(_mutex);
            _messages.push_back("warning: " + message);
            if (_started)
            {
                dump();
            }
        }

        void error(const string& message) final
        {
            lock_guard lock(_mutex);
            _messages.push_back("error: " + message);
            if (_started)
            {
                dump();
            }
        }

        string getPrefix() final { return ""; }

        Ice::LoggerPtr cloneWithPrefix(const string&) final { return shared_from_this(); }

    private:
        void dump()
        {
            for (const auto& message : _messages)
            {
                cout << message << endl;
            }
            _messages.clear();
        }

        bool _started;
        vector<string> _messages;
        mutex _mutex;
    };
    using LoggerIPtr = shared_ptr<LoggerI>;

    class TestCase : public enable_shared_from_this<TestCase>
    {
    public:
        TestCase(const string& name, const RemoteCommunicatorPrx& com)
            : _testCaseName(name),
              _com(com),
              _logger(new LoggerI()),
              _clientACMTimeout(-1),
              _clientACMClose(-1),
              _clientACMHeartbeat(-1),
              _serverACMTimeout(-1),
              _serverACMClose(-1),
              _serverACMHeartbeat(-1),
              _heartbeat(0),
              _closed(false)
        {
        }

        void init()
        {
            _adapter = _com->createObjectAdapter(_serverACMTimeout, _serverACMClose, _serverACMHeartbeat);

            Ice::InitializationData initData;
            initData.properties = _com->ice_getCommunicator()->getProperties()->clone();
            initData.logger = _logger;
            initData.properties->setProperty("Ice.ACM.Timeout", "2");
            if (_clientACMTimeout >= 0)
            {
                initData.properties->setProperty("Ice.ACM.Client.Timeout", toString(_clientACMTimeout));
            }
            if (_clientACMClose >= 0)
            {
                initData.properties->setProperty("Ice.ACM.Client.Close", toString(_clientACMClose));
            }
            if (_clientACMHeartbeat >= 0)
            {
                initData.properties->setProperty("Ice.ACM.Client.Heartbeat", toString(_clientACMHeartbeat));
            }
            _communicator = Ice::initialize(initData);
        }

        void destroy()
        {
            _adapter->deactivate();
            _communicator->destroy();
        }

        void join(thread& t)
        {
            cout << "testing " << _testCaseName << "... " << flush;
            _logger->start();
            t.join();
            if (_msg.empty())
            {
                cout << "ok" << endl;
            }
            else
            {
                cout << "failed! " << endl << _msg;
                test(false);
            }
        }

        void run()
        {
            auto proxy = TestIntfPrx(_communicator, _adapter->getTestIntf()->ice_toString());
            try
            {
                auto self = shared_from_this();
                proxy->ice_getConnection()->setCloseCallback([self](Ice::ConnectionPtr connection)
                                                             { self->closed(std::move(connection)); });
                proxy->ice_getConnection()->setHeartbeatCallback([self](Ice::ConnectionPtr connection)
                                                                 { self->heartbeat(std::move(connection)); });
                runTestCase(_adapter.value(), proxy);
            }
            catch (const exception& ex)
            {
                _msg = string("unexpected exception:\n") + ex.what();
            }
            catch (...)
            {
                _msg = "unknown exception";
            }
        }

        void heartbeat(const Ice::ConnectionPtr&)
        {
            lock_guard lock(_mutex);
            ++_heartbeat;
        }

        void closed(const Ice::ConnectionPtr&)
        {
            lock_guard lock(_mutex);
            _closed = true;
            _conditionVariable.notify_one();
        }

        void waitForClosed()
        {
            unique_lock lock(_mutex);
            if (!_conditionVariable.wait_for(lock, chrono::seconds(30), [this] { return _closed; }))
            {
                test(false); // Waited for more than 30s for close, something's wrong.
            }
        }

        virtual void runTestCase(const RemoteObjectAdapterPrx&, const TestIntfPrx&) = 0;

        void setClientACM(int timeout, int close, int heartbeat)
        {
            _clientACMTimeout = timeout;
            _clientACMClose = close;
            _clientACMHeartbeat = heartbeat;
        }

        void setServerACM(int timeout, int close, int heartbeat)
        {
            _serverACMTimeout = timeout;
            _serverACMClose = close;
            _serverACMHeartbeat = heartbeat;
        }

    protected:
        const string _testCaseName;
        const RemoteCommunicatorPrx _com;
        string _msg;
        LoggerIPtr _logger;

        Ice::CommunicatorPtr _communicator;
        optional<RemoteObjectAdapterPrx> _adapter;

        int _clientACMTimeout;
        int _clientACMClose;
        int _clientACMHeartbeat;
        int _serverACMTimeout;
        int _serverACMClose;
        int _serverACMHeartbeat;

        int _heartbeat;
        bool _closed;
        mutex _mutex;
        condition_variable _conditionVariable;
    };
    using TestCasePtr = shared_ptr<TestCase>;

    class InvocationHeartbeatTest final : public TestCase
    {
    public:
        InvocationHeartbeatTest(const RemoteCommunicatorPrx& com) : TestCase("invocation heartbeat", com)
        {
            setServerACM(1, -1, -1); // Faster ACM to make sure we receive enough ACM heartbeats
        }

        void runTestCase(const RemoteObjectAdapterPrx&, const TestIntfPrx& proxy) final
        {
            proxy->sleep(4);

            lock_guard lock(_mutex);
            test(_heartbeat >= 4);
        }
    };

    class InvocationHeartbeatOnHoldTest final : public TestCase
    {
    public:
        InvocationHeartbeatOnHoldTest(const RemoteCommunicatorPrx& com)
            : TestCase("invocation with heartbeat on hold", com)
        {
            // Use default ACM configuration.
        }

        void runTestCase(const RemoteObjectAdapterPrx& adapter, const TestIntfPrx& proxy) final
        {
            try
            {
                // When the OA is put on hold, connections shouldn't
                // send heartbeats, the invocation should therefore
                // fail.
                proxy->sleepAndHold(10);
                test(false);
            }
            catch (const Ice::ConnectionTimeoutException&)
            {
                adapter->activate();
                proxy->interruptSleep();

                waitForClosed();
            }
        }
    };

    class InvocationNoHeartbeatTest final : public TestCase
    {
    public:
        InvocationNoHeartbeatTest(const RemoteCommunicatorPrx& com) : TestCase("invocation with no heartbeat", com)
        {
            setServerACM(2, 2, 0); // Disable heartbeat on invocations
        }

        void runTestCase(const RemoteObjectAdapterPrx&, const TestIntfPrx& proxy) final
        {
            try
            {
                // Heartbeats are disabled on the server, the
                // invocation should fail since heartbeats are
                // expected.
                proxy->sleep(10);
                test(false);
            }
            catch (const Ice::ConnectionTimeoutException&)
            {
                proxy->interruptSleep();

                waitForClosed();

                lock_guard lock(_mutex);
                test(_heartbeat == 0);
            }
        }
    };

    class InvocationHeartbeatCloseOnIdleTest final : public TestCase
    {
    public:
        InvocationHeartbeatCloseOnIdleTest(const RemoteCommunicatorPrx& com)
            : TestCase("invocation with no heartbeat and close on idle", com)
        {
            setClientACM(1, 1, 0); // Only close on idle.
            setServerACM(1, 2, 0); // Disable heartbeat on invocations
        }

        void runTestCase(const RemoteObjectAdapterPrx&, const TestIntfPrx& proxy) final
        {
            // No close on invocation, the call should succeed this time.
            proxy->sleep(3);

            lock_guard lock(_mutex);
            test(_heartbeat == 0);
            test(!_closed);
        }
    };

    class CloseOnIdleTest final : public TestCase
    {
    public:
        CloseOnIdleTest(const RemoteCommunicatorPrx& com) : TestCase("close on idle", com)
        {
            setClientACM(1, 1, 0); // Only close on idle
        }

        void runTestCase(const RemoteObjectAdapterPrx&, const TestIntfPrx&) final
        {
            waitForClosed();

            lock_guard lock(_mutex);
            test(_heartbeat == 0);
        }
    };

    class CloseOnInvocationTest final : public TestCase
    {
    public:
        CloseOnInvocationTest(const RemoteCommunicatorPrx& com) : TestCase("close on invocation", com)
        {
            setClientACM(1, 2, 0); // Only close on invocation
        }

        void runTestCase(const RemoteObjectAdapterPrx&, const TestIntfPrx&) final
        {
            this_thread::sleep_for(chrono::milliseconds(3000)); // Idle for 3 seconds

            lock_guard lock(_mutex);
            test(_heartbeat == 0);
            test(!_closed);
        }
    };

    class CloseOnIdleAndInvocationTest final : public TestCase
    {
    public:
        CloseOnIdleAndInvocationTest(const RemoteCommunicatorPrx& com) : TestCase("close on idle and invocation", com)
        {
            setClientACM(3, 3, 0); // Only close on idle and invocation
        }

        void runTestCase(const RemoteObjectAdapterPrx& adapter, const TestIntfPrx&) final
        {
            //
            // Put the adapter on hold. The server will not respond to
            // the graceful close. This allows to test whether or not
            // the close is graceful or forceful.
            //
            adapter->hold();
            this_thread::sleep_for(chrono::milliseconds(5000)); // Idle for 5 seconds

            {
                lock_guard lock(_mutex);
                test(_heartbeat == 0);
                test(!_closed); // Not closed yet because of graceful close.
            }

            adapter->activate();

            waitForClosed();
        }
    };

    class ForcefulCloseOnIdleAndInvocationTest final : public TestCase
    {
    public:
        ForcefulCloseOnIdleAndInvocationTest(const RemoteCommunicatorPrx& com)
            : TestCase("forceful close on idle and invocation", com)
        {
            setClientACM(1, 4, 0); // Only close on idle and invocation
        }

        void runTestCase(const RemoteObjectAdapterPrx& adapter, const TestIntfPrx&) final
        {
            adapter->hold();

            waitForClosed();

            lock_guard lock(_mutex);
            test(_heartbeat == 0);
        }
    };

    class HeartbeatOnIdleTest final : public TestCase
    {
    public:
        HeartbeatOnIdleTest(const RemoteCommunicatorPrx& com) : TestCase("heartbeat on idle", com)
        {
            setServerACM(1, -1, 2); // Enable server heartbeats.
        }

        void runTestCase(const RemoteObjectAdapterPrx&, const TestIntfPrx&) final
        {
            this_thread::sleep_for(chrono::milliseconds(3000));

            lock_guard lock(_mutex);
            test(_heartbeat >= 3);
        }
    };

    class HeartbeatAlwaysTest final : public TestCase
    {
    public:
        HeartbeatAlwaysTest(const RemoteCommunicatorPrx& com) : TestCase("heartbeat always", com)
        {
            setServerACM(1, -1, 3); // Enable server heartbeats.
        }

        void runTestCase(const RemoteObjectAdapterPrx&, const TestIntfPrx& proxy) final
        {
            for (int i = 0; i < 10; ++i)
            {
                proxy->ice_ping();
                this_thread::sleep_for(chrono::milliseconds(300));
            }

            lock_guard lock(_mutex);
            test(_heartbeat >= 3);
        }
    };

    class HeartbeatManualTest final : public TestCase
    {
    public:
        HeartbeatManualTest(const RemoteCommunicatorPrx& com) : TestCase("manual heartbeats", com)
        {
            //
            // Disable heartbeats.
            //
            setClientACM(10, -1, 0);
            setServerACM(10, -1, 0);
        }

        void runTestCase(const RemoteObjectAdapterPrx&, const TestIntfPrx& proxy) final
        {
            proxy->startHeartbeatCount();
            Ice::ConnectionPtr con = proxy->ice_getConnection();
            con->heartbeat();
            con->heartbeat();
            con->heartbeat();
            con->heartbeat();
            con->heartbeat();
            proxy->waitForHeartbeatCount(5);
        }
    };

    class SetACMTest final : public TestCase
    {
    public:
        SetACMTest(const RemoteCommunicatorPrx& com) : TestCase("setACM/getACM", com) { setClientACM(15, 4, 0); }

        void runTestCase(const RemoteObjectAdapterPrx&, const TestIntfPrx& proxy) final
        {
            Ice::ConnectionPtr con = proxy->ice_getConnection();

            try
            {
                con->setACM(-19, nullopt, nullopt);
                test(false);
            }
            catch (const invalid_argument&)
            {
            }

            Ice::ACM acm;
            acm = con->getACM();
            test(acm.timeout == 15);
            test(acm.close == Ice::ACMClose::CloseOnIdleForceful);
            test(acm.heartbeat == Ice::ACMHeartbeat::HeartbeatOff);

            con->setACM(nullopt, nullopt, nullopt);
            acm = con->getACM();
            test(acm.timeout == 15);
            test(acm.close == Ice::ACMClose::CloseOnIdleForceful);
            test(acm.heartbeat == Ice::ACMHeartbeat::HeartbeatOff);

            con->setACM(1, Ice::ACMClose::CloseOnInvocationAndIdle, Ice::ACMHeartbeat::HeartbeatAlways);
            acm = con->getACM();
            test(acm.timeout == 1);
            test(acm.close == Ice::ACMClose::CloseOnInvocationAndIdle);
            test(acm.heartbeat == Ice::ACMHeartbeat::HeartbeatAlways);

            // Make sure the client sends a few heartbeats to the server.
            proxy->startHeartbeatCount();
            proxy->waitForHeartbeatCount(2);

            auto p1 = promise<void>();
            con->setCloseCallback([&p1](shared_ptr<Ice::Connection>) { p1.set_value(); });

            con->close(Ice::ConnectionClose::Gracefully);
            p1.get_future().wait();

            try
            {
                con->throwException();
                test(false);
            }
            catch (const Ice::ConnectionManuallyClosedException&)
            {
            }

            auto p2 = promise<void>();
            con->setCloseCallback([&p2](shared_ptr<Ice::Connection>) { p2.set_value(); });
            p2.get_future().wait();

            con->setHeartbeatCallback([](shared_ptr<Ice::Connection>) {});
        }
    };
}

void
allTests(Test::TestHelper* helper)
{
    Ice::CommunicatorPtr communicator = helper->communicator();
    RemoteCommunicatorPrx com(communicator, "communicator:" + helper->getTestEndpoint());

    vector<TestCasePtr> tests;

    tests.push_back(make_shared<InvocationHeartbeatTest>(com));
    tests.push_back(make_shared<InvocationHeartbeatOnHoldTest>(com));
    tests.push_back(make_shared<InvocationNoHeartbeatTest>(com));
    tests.push_back(make_shared<InvocationHeartbeatCloseOnIdleTest>(com));

    tests.push_back(make_shared<CloseOnIdleTest>(com));
    tests.push_back(make_shared<CloseOnInvocationTest>(com));
    tests.push_back(make_shared<CloseOnIdleAndInvocationTest>(com));
    tests.push_back(make_shared<ForcefulCloseOnIdleAndInvocationTest>(com));

    tests.push_back(make_shared<HeartbeatOnIdleTest>(com));
    tests.push_back(make_shared<HeartbeatAlwaysTest>(com));
    tests.push_back(make_shared<HeartbeatManualTest>(com));
    tests.push_back(make_shared<SetACMTest>(com));

    for (const auto& testCase : tests)
    {
        testCase->init();
    }

    vector<pair<thread, TestCasePtr>> threads;
    for (const auto& testCase : tests)
    {
        thread thread([testCase]() { testCase->run(); });
        threads.push_back(make_pair(std::move(thread), testCase));
    }

    for (auto& [thread, testCase] : threads)
    {
        testCase->join(thread);
    }

    for (const auto& testCase : tests)
    {
        testCase->destroy();
    }

    cout << "shutting down... " << flush;
    com->shutdown();
    cout << "ok" << endl;
}
