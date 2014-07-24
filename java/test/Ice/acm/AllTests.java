// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.acm;

import java.io.PrintWriter;

import test.Ice.acm.Test.RemoteCommunicatorPrx;
import test.Ice.acm.Test.RemoteCommunicatorPrxHelper;
import test.Ice.acm.Test.RemoteObjectAdapterPrx;
import test.Ice.acm.Test.TestIntfPrx;
import test.Ice.acm.Test.TestIntfPrxHelper;

public class AllTests
{
    private static void
    test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    static class LoggerI implements Ice.Logger
    {
        LoggerI(java.io.PrintWriter out)
        {
            _out = out;
        }

        public void start()
        {
            synchronized(this)
            {
                _started = true;
                dump();
            }
        }

        public void print(String msg)
        {
            synchronized(this)
            {
                _messages.add(msg);
                if(_started)
                {
                    dump();
                }
            }
        }

        public void trace(String category, String message)
        {
            synchronized(this)
            {
                _messages.add("[" + category + "] " + message);
                if(_started)
                {
                    dump();
                }
            }
        }

        public void warning(String message)
        {
            synchronized(this)
            {
                _messages.add("warning: " + message);
                if(_started)
                {
                    dump();
                }
            }
        }

        public void error(String message)
        {
            synchronized(this)
            {
                _messages.add("error: " + message);
                if(_started)
                {
                    dump();
                }
            }
        }

        public Ice.Logger cloneWithPrefix(String prefix)
        {
            return this;
        }

        private void dump()
        {
            for(String line : _messages)
            {
                _out.println(line);
            }
            _messages.clear();
        }

        private boolean _started;
        private java.util.List<String> _messages = new java.util.ArrayList<String>();
        private java.io.PrintWriter _out;
    };

    static abstract class TestCase implements Ice.ConnectionCallback
    {
        public TestCase(String name, RemoteCommunicatorPrx com, java.io.PrintWriter out)
        {
            _name = name;
            _com = com;
            _logger = new LoggerI(out);

            _clientACMTimeout = -1;
            _clientACMClose = -1;
            _clientACMHeartbeat = -1;

            _serverACMTimeout = -1;
            _serverACMClose = -1;
            _serverACMHeartbeat = -1;

            _heartbeat = 0;
            _closed = false;
        }

        public void init()
        {
            _adapter = _com.createObjectAdapter(_serverACMTimeout, _serverACMClose, _serverACMHeartbeat);

            Ice.InitializationData initData = new Ice.InitializationData();
            initData.properties = _com.ice_getCommunicator().getProperties()._clone();
            initData.logger = _logger;
            initData.properties.setProperty("Ice.ACM.Timeout", "1");
            if(_clientACMTimeout >= 0)
            {
                initData.properties.setProperty("Ice.ACM.Client.Timeout", Integer.toString(_clientACMTimeout));
            }
            if(_clientACMClose >= 0)
            {
                initData.properties.setProperty("Ice.ACM.Client.Close", Integer.toString(_clientACMClose));
            }
            if(_clientACMHeartbeat >= 0)
            {
                initData.properties.setProperty("Ice.ACM.Client.Heartbeat", Integer.toString(_clientACMHeartbeat));
            }
            //initData.properties.setProperty("Ice.Trace.Protocol", "2");
            //initData.properties.setProperty("Ice.Trace.Network", "2");
            _communicator = Ice.Util.initialize(initData);

            _thread = new Thread(
                new Runnable()
                {
                    public void
                    run()
                    {
                        TestCase.this.run();
                    }
                });
        }

        public void start()
        {
            _thread.start();
        }

        public void destroy()
        {
            _adapter.deactivate();
            _communicator.destroy();
        }

        public void join(PrintWriter out)
        {
            out.print("testing " + _name + "... ");
            out.flush();
            _logger.start();
            try
            {
                _thread.join();
            }
            catch(java.lang.InterruptedException ex)
            {
                assert(false);
            }
            if(_msg == null)
            {
                out.println("ok");
            }
            else
            {
                out.println("failed! " + _msg);
                test(false);
            }
        }

        public void run()
        {
            TestIntfPrx proxy = TestIntfPrxHelper.uncheckedCast(_communicator.stringToProxy(
                                                                    _adapter.getTestIntf().toString()));
            try
            {
                proxy.ice_getConnection().setCallback(this);
                runTestCase(_adapter, proxy);
            }
            catch(Exception ex)
            {
                _msg = "unexpected exception:\n" + ex.toString();
            }
        }

        synchronized public void heartbeat(Ice.Connection con)
        {
            ++_heartbeat;
        }

        synchronized public void closed(Ice.Connection con)
        {
            _closed = true;
        }

        public abstract void runTestCase(RemoteObjectAdapterPrx adapter, TestIntfPrx proxy);

        public void setClientACM(int timeout, int close, int heartbeat)
        {
            _clientACMTimeout = timeout;
            _clientACMClose = close;
            _clientACMHeartbeat = heartbeat;
        }

        public void  setServerACM(int timeout, int close, int heartbeat)
        {
            _serverACMTimeout = timeout;
            _serverACMClose = close;
            _serverACMHeartbeat = heartbeat;
        }

        private String _name;
        private RemoteCommunicatorPrx _com;
        private String _msg;
        private LoggerI _logger;
        private Thread _thread;

        private Ice.Communicator _communicator;
        private RemoteObjectAdapterPrx _adapter;

        private int _clientACMTimeout;
        private int _clientACMClose;
        private int _clientACMHeartbeat;
        private int _serverACMTimeout;
        private int _serverACMClose;
        private int _serverACMHeartbeat;

        protected int _heartbeat;
        protected boolean _closed;
    };

    static class InvocationHeartbeatTest extends TestCase
    {
        public InvocationHeartbeatTest(RemoteCommunicatorPrx com, java.io.PrintWriter out)
        {
            super("invocation heartbeat", com, out);
        }

        public void runTestCase(RemoteObjectAdapterPrx adapter, TestIntfPrx proxy)
        {
            proxy.sleep(2);
            test(_heartbeat >= 2);
        }
    };

    static class InvocationHeartbeatOnHoldTest extends TestCase
    {
        public InvocationHeartbeatOnHoldTest(RemoteCommunicatorPrx com, java.io.PrintWriter out)
        {
            super("invocation with heartbeat on hold", com, out);
            // Use default ACM configuration.
        }

        public void runTestCase(RemoteObjectAdapterPrx adapter, TestIntfPrx proxy)
        {
            try
            {
                // When the OA is put on hold, connections shouldn't
                // send heartbeats, the invocation should therefore
                // fail.
                proxy.sleepAndHold(10);
                test(false);
            }
            catch(Ice.ConnectionTimeoutException ex)
            {
                adapter.activate();
                proxy.interruptSleep();

                synchronized(this)
                {
                    test(_closed);
                }
            }
        }
    };

    static class InvocationNoHeartbeatTest extends TestCase
    {
        public InvocationNoHeartbeatTest(RemoteCommunicatorPrx com, java.io.PrintWriter out)
        {
            super("invocation with no heartbeat", com, out);
            setServerACM(1, 2, 0); // Disable heartbeat on invocations
        }

        public void runTestCase(RemoteObjectAdapterPrx adapter, TestIntfPrx proxy)
        {
            try
            {
                // Heartbeats are disabled on the server, the
                // invocation should fail since heartbeats are
                // expected.
                proxy.sleep(10);
                test(false);
            }
            catch(Ice.ConnectionTimeoutException ex)
            {
                proxy.interruptSleep();

                synchronized(this)
                {
                    test(_heartbeat == 0);
                    test(_closed);
                }
            }
        }
    };

    static class InvocationHeartbeatCloseOnIdleTest extends TestCase
    {
        public InvocationHeartbeatCloseOnIdleTest(RemoteCommunicatorPrx com, java.io.PrintWriter out)
        {
            super("invocation with no heartbeat and close on idle", com, out);
            setClientACM(1, 1, 0); // Only close on idle.
            setServerACM(1, 2, 0); // Disable heartbeat on invocations
        }

        public void runTestCase(RemoteObjectAdapterPrx adapter, TestIntfPrx proxy)
        {
            // No close on invocation, the call should succeed this
            // time.
            proxy.sleep(2);

            synchronized(this)
            {
                test(_heartbeat == 0);
                test(!_closed);
            }
        }
    };

    static class CloseOnIdleTest extends TestCase
    {
        public CloseOnIdleTest(RemoteCommunicatorPrx com, java.io.PrintWriter out)
        {
            super("close on idle", com, out);
            setClientACM(1, 1, 0); // Only close on idle
        }

        public void runTestCase(RemoteObjectAdapterPrx adapter, TestIntfPrx proxy)
        {
            try
            {
                Thread.sleep(1500); // Idle for 1.5 second
            }
            catch(java.lang.InterruptedException ex)
            {
            }

            synchronized(this)
            {
                test(_heartbeat == 0);
                test(_closed);
            }
        }
    };

    static class CloseOnInvocationTest extends TestCase
    {
        public CloseOnInvocationTest(RemoteCommunicatorPrx com, java.io.PrintWriter out)
        {
            super("close on invocation", com, out);
            setClientACM(1, 2, 0); // Only close on invocation
        }

        public void runTestCase(RemoteObjectAdapterPrx adapter, TestIntfPrx proxy)
        {
            try
            {
                Thread.sleep(1500); // Idle for 1.5 second
            }
            catch(java.lang.InterruptedException ex)
            {
            }

            synchronized(this)
            {
                test(_heartbeat == 0);
                test(!_closed);
            }
        }
    };

    static class CloseOnIdleAndInvocationTest extends TestCase
    {
        public CloseOnIdleAndInvocationTest(RemoteCommunicatorPrx com, java.io.PrintWriter out)
        {
            super("close on idle and invocation", com, out);
            setClientACM(1, 3, 0); // Only close on idle and invocation
        }

        public void runTestCase(RemoteObjectAdapterPrx adapter, TestIntfPrx proxy)
        {
            //
            // Put the adapter on hold. The server will not respond to
            // the graceful close. This allows to test whether or not
            // the close is graceful or forceful.
            //
            adapter.hold();
            try
            {
                Thread.sleep(1500); // Idle for 1.5 second
            }
            catch(java.lang.InterruptedException ex)
            {
            }

            synchronized(this)
            {
                test(_heartbeat == 0);
                test(!_closed); // Not closed yet because of graceful close.
            }

            adapter.activate();
            try
            {
                Thread.sleep(500);
            }
            catch(java.lang.InterruptedException ex)
            {
            }

            synchronized(this)
            {
                test(_closed); // Connection should be closed this time.
            }
        }
    };

    static class ForcefulCloseOnIdleAndInvocationTest extends TestCase
    {
        public ForcefulCloseOnIdleAndInvocationTest(RemoteCommunicatorPrx com, java.io.PrintWriter out)
        {
            super("forceful close on idle and invocation", com, out);
            setClientACM(1, 4, 0); // Only close on idle and invocation
        }

        public void runTestCase(RemoteObjectAdapterPrx adapter, TestIntfPrx proxy)
        {
            adapter.hold();
            try
            {
                Thread.sleep(1500); // Idle for 1.5 second
            }
            catch(java.lang.InterruptedException ex)
            {
            }

            synchronized(this)
            {
                test(_heartbeat == 0);
                test(_closed); // Connection closed forcefully by ACM
            }
        }
    };

    static class HeartbeatOnIdleTest extends TestCase
    {
        public HeartbeatOnIdleTest(RemoteCommunicatorPrx com, java.io.PrintWriter out)
        {
            super("heartbeat on idle", com, out);
            setServerACM(1, -1, 2); // Enable server heartbeats.
        }

        public void runTestCase(RemoteObjectAdapterPrx adapter, TestIntfPrx proxy)
        {
            try
            {
                Thread.sleep(2000);
            }
            catch(java.lang.InterruptedException ex)
            {
            }

            synchronized(this)
            {
                test(_heartbeat >= 3);
            }
        }
    };

    static class HeartbeatAlwaysTest extends TestCase
    {
        public HeartbeatAlwaysTest(RemoteCommunicatorPrx com, java.io.PrintWriter out)
        {
            super("heartbeat always", com, out);
            setServerACM(1, -1, 3); // Enable server heartbeats.
        }

        public void runTestCase(RemoteObjectAdapterPrx adapter, TestIntfPrx proxy)
        {
            for(int i = 0; i < 12; i++)
            {
                proxy.ice_ping();
                try
                {
                    Thread.sleep(200);
                }
                catch(java.lang.InterruptedException ex)
                {
                }
            }

            synchronized(this)
            {
                test(_heartbeat >= 3);
            }
        }
    };

    static class SetACMTest extends TestCase
    {
        public SetACMTest(RemoteCommunicatorPrx com, java.io.PrintWriter out)
        {
            super("setACM/getACM", com, out);
            setClientACM(15, 4, 2);
        }

        public void runTestCase(RemoteObjectAdapterPrx adapter, TestIntfPrx proxy)
        {
            Ice.ACM acm = new Ice.ACM();
            acm = proxy.ice_getCachedConnection().getACM();
            test(acm.timeout == 15);
            test(acm.close == Ice.ACMClose.CloseOnIdleForceful);
            test(acm.heartbeat == Ice.ACMHeartbeat.HeartbeatOnIdle);

            proxy.ice_getCachedConnection().setACM(null, null, null);
            acm = proxy.ice_getCachedConnection().getACM();
            test(acm.timeout == 15);
            test(acm.close == Ice.ACMClose.CloseOnIdleForceful);
            test(acm.heartbeat == Ice.ACMHeartbeat.HeartbeatOnIdle);

            proxy.ice_getCachedConnection().setACM(
                new Ice.IntOptional(20),
                new Ice.Optional<Ice.ACMClose>(Ice.ACMClose.CloseOnInvocationAndIdle),
                new Ice.Optional<Ice.ACMHeartbeat>(Ice.ACMHeartbeat.HeartbeatOnInvocation));
            acm = proxy.ice_getCachedConnection().getACM();
            test(acm.timeout == 20);
            test(acm.close == Ice.ACMClose.CloseOnInvocationAndIdle);
            test(acm.heartbeat == Ice.ACMHeartbeat.HeartbeatOnInvocation);
        }
    };

    public static void
    allTests(Ice.Communicator communicator, PrintWriter out)
    {
        String ref = "communicator:default -p 12010";
        RemoteCommunicatorPrx com = RemoteCommunicatorPrxHelper.uncheckedCast(communicator.stringToProxy(ref));

        java.util.List<TestCase> tests = new java.util.ArrayList<TestCase>();

        tests.add(new InvocationHeartbeatTest(com, out));
        tests.add(new InvocationHeartbeatOnHoldTest(com, out));
        tests.add(new InvocationNoHeartbeatTest(com, out));
        tests.add(new InvocationHeartbeatCloseOnIdleTest(com, out));

        tests.add(new CloseOnIdleTest(com, out));
        tests.add(new CloseOnInvocationTest(com, out));
        tests.add(new CloseOnIdleAndInvocationTest(com, out));
        tests.add(new ForcefulCloseOnIdleAndInvocationTest(com, out));

        tests.add(new HeartbeatOnIdleTest(com, out));
        tests.add(new HeartbeatAlwaysTest(com, out));
        tests.add(new SetACMTest(com, out));

        for(TestCase test : tests)
        {
            test.init();
        }
        for(TestCase test : tests)
        {
            test.start();
        }
        for(TestCase test : tests)
        {
            test.join(out);
        }
        for(TestCase test : tests)
        {
            test.destroy();
        }

        out.print("shutting down... ");
        out.flush();
        com.shutdown();
        out.println("ok");
    }
}
