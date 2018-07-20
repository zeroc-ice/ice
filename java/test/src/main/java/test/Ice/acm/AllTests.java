// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.acm;

import java.io.PrintWriter;
import java.util.concurrent.CompletableFuture;

import test.Ice.acm.Test.RemoteCommunicatorPrx;
import test.Ice.acm.Test.RemoteObjectAdapterPrx;
import test.Ice.acm.Test.TestIntfPrx;
import test.TestHelper;

import com.zeroc.Ice.ACMClose;
import com.zeroc.Ice.ACMHeartbeat;

public class AllTests
{
    private static com.zeroc.Ice.Communicator communicator;

    private static void test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    static class LoggerI implements com.zeroc.Ice.Logger
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

        public String getPrefix()
        {
            return "";
        }

        public com.zeroc.Ice.Logger cloneWithPrefix(String prefix)
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
        private java.util.List<String> _messages = new java.util.ArrayList<>();
        private java.io.PrintWriter _out;
    }

    static abstract class TestCase
    {
        public TestCase(TestHelper helper, String name, RemoteCommunicatorPrx com, PrintWriter out)
        {
            _helper = helper;
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

            com.zeroc.Ice.InitializationData initData = new com.zeroc.Ice.InitializationData();
            initData.properties = _helper.communicator().getProperties()._clone();
            initData.logger = _logger;
            initData.properties.setProperty("Ice.ACM.Timeout", "2");
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
            // initData.properties.setProperty("Ice.Trace.Protocol", "2");
            // initData.properties.setProperty("Ice.Trace.Network", "2");
            _communicator = _helper.initialize(initData);

            _thread = new Thread(() -> { TestCase.this.run(); });
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
            TestIntfPrx proxy = TestIntfPrx.uncheckedCast(_communicator.stringToProxy(
                _adapter.getTestIntf().toString()));

            try
            {
                proxy.ice_getConnection().setCloseCallback(con ->
                    {
                        synchronized(TestCase.this)
                        {
                            _closed = true;
                            TestCase.this.notify();
                        }
                    });

                proxy.ice_getConnection().setHeartbeatCallback(con ->
                    {
                        synchronized(TestCase.this)
                        {
                            ++_heartbeat;
                        }
                    });

                runTestCase(_adapter, proxy);
            }
            catch(Exception ex)
            {
                _msg = "unexpected exception:\n" + com.zeroc.IceInternal.Ex.toString(ex);
            }
        }

        public synchronized void waitForClosed()
        {
            while(!_closed)
            {
                long now = com.zeroc.IceInternal.Time.currentMonotonicTimeMillis();
                try
                {
                    wait(2000);
                    if(com.zeroc.IceInternal.Time.currentMonotonicTimeMillis() - now > 2000)
                    {
                        test(false); // Waited for more than 2s for close, something's wrong.
                    }
                }
                catch(java.lang.InterruptedException ex)
                {
                }
            }
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

        private final TestHelper _helper;
        private String _name;
        private RemoteCommunicatorPrx _com;
        private String _msg;
        private LoggerI _logger;
        private Thread _thread;

        private com.zeroc.Ice.Communicator _communicator;
        private RemoteObjectAdapterPrx _adapter;

        private int _clientACMTimeout;
        private int _clientACMClose;
        private int _clientACMHeartbeat;
        private int _serverACMTimeout;
        private int _serverACMClose;
        private int _serverACMHeartbeat;

        protected int _heartbeat;
        protected boolean _closed;
    }

    static class InvocationHeartbeatTest extends TestCase
    {
        public InvocationHeartbeatTest(TestHelper helper, RemoteCommunicatorPrx com, java.io.PrintWriter out)
        {
            super(helper, "invocation heartbeat", com, out);
            setServerACM(1, -1, -1); // Faster ACM to make sure we receive enough ACM heartbeats
        }

        public void runTestCase(RemoteObjectAdapterPrx adapter, TestIntfPrx proxy)
        {
            proxy.sleep(4);
            test(_heartbeat >= 4);
        }
    }

    static class InvocationHeartbeatOnHoldTest extends TestCase
    {
        public InvocationHeartbeatOnHoldTest(TestHelper helper, RemoteCommunicatorPrx com, java.io.PrintWriter out)
        {
            super(helper, "invocation with heartbeat on hold", com, out);
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
            catch(com.zeroc.Ice.ConnectionTimeoutException ex)
            {
                adapter.activate();
                proxy.interruptSleep();
                waitForClosed();
            }
        }
    }

    static class InvocationNoHeartbeatTest extends TestCase
    {
        public InvocationNoHeartbeatTest(TestHelper helper, RemoteCommunicatorPrx com, java.io.PrintWriter out)
        {
            super(helper, "invocation with no heartbeat", com, out);
            setServerACM(2, 2, 0); // Disable heartbeat on invocations
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
            catch(com.zeroc.Ice.ConnectionTimeoutException ex)
            {
                proxy.interruptSleep();
                waitForClosed();

                synchronized(this)
                {
                    test(_heartbeat == 0);
                }
            }
        }
    }

    static class InvocationHeartbeatCloseOnIdleTest extends TestCase
    {
        public InvocationHeartbeatCloseOnIdleTest(TestHelper helper, RemoteCommunicatorPrx com, java.io.PrintWriter out)
        {
            super(helper, "invocation with no heartbeat and close on idle", com, out);
            setClientACM(1, 1, 0); // Only close on idle.
            setServerACM(1, 2, 0); // Disable heartbeat on invocations
        }

        public void runTestCase(RemoteObjectAdapterPrx adapter, TestIntfPrx proxy)
        {
            // No close on invocation, the call should succeed this
            // time.
            proxy.sleep(3);

            synchronized(this)
            {
                test(_heartbeat == 0);
                test(!_closed);
            }
        }
    }

    static class CloseOnIdleTest extends TestCase
    {
        public CloseOnIdleTest(TestHelper helper, RemoteCommunicatorPrx com, java.io.PrintWriter out)
        {
            super(helper, "close on idle", com, out);
            setClientACM(1, 1, 0); // Only close on idle
        }

        public void runTestCase(RemoteObjectAdapterPrx adapter, TestIntfPrx proxy)
        {
            try
            {
                Thread.sleep(3000); // Idle for 3 seconds
            }
            catch(java.lang.InterruptedException ex)
            {
            }

            waitForClosed();

            synchronized(this)
            {
                test(_heartbeat == 0);
            }
        }
    }

    static class CloseOnInvocationTest extends TestCase
    {
        public CloseOnInvocationTest(TestHelper helper, RemoteCommunicatorPrx com, java.io.PrintWriter out)
        {
            super(helper, "close on invocation", com, out);
            setClientACM(1, 2, 0); // Only close on invocation
        }

        public void runTestCase(RemoteObjectAdapterPrx adapter, TestIntfPrx proxy)
        {
            try
            {
                Thread.sleep(3000); // Idle for 3 seconds
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
    }

    static class CloseOnIdleAndInvocationTest extends TestCase
    {
        public CloseOnIdleAndInvocationTest(TestHelper helper, RemoteCommunicatorPrx com, java.io.PrintWriter out)
        {
            super(helper, "close on idle and invocation", com, out);
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
                Thread.sleep(3000); // Idle for 3 seconds
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
                Thread.sleep(1000);
            }
            catch(java.lang.InterruptedException ex)
            {
            }

            waitForClosed();
        }
    }

    static class ForcefulCloseOnIdleAndInvocationTest extends TestCase
    {
        public ForcefulCloseOnIdleAndInvocationTest(TestHelper helper, RemoteCommunicatorPrx com, java.io.PrintWriter out)
        {
            super(helper, "forceful close on idle and invocation", com, out);
            setClientACM(1, 4, 0); // Only close on idle and invocation
        }

        public void runTestCase(RemoteObjectAdapterPrx adapter, TestIntfPrx proxy)
        {
            adapter.hold();
            try
            {
                Thread.sleep(3000); // Idle for 3 seconds
            }
            catch(java.lang.InterruptedException ex)
            {
            }
            waitForClosed();
            synchronized(this)
            {
                test(_heartbeat == 0);
            }
        }
    }

    static class HeartbeatOnIdleTest extends TestCase
    {
        public HeartbeatOnIdleTest(TestHelper helper, RemoteCommunicatorPrx com, java.io.PrintWriter out)
        {
            super(helper, "heartbeat on idle", com, out);
            setServerACM(1, -1, 2); // Enable server heartbeats.
        }

        public void runTestCase(RemoteObjectAdapterPrx adapter, TestIntfPrx proxy)
        {
            try
            {
                Thread.sleep(3000);
            }
            catch(java.lang.InterruptedException ex)
            {
            }

            synchronized(this)
            {
                test(_heartbeat >= 3);
            }
        }
    }

    static class HeartbeatAlwaysTest extends TestCase
    {
        public HeartbeatAlwaysTest(TestHelper helper, RemoteCommunicatorPrx com, java.io.PrintWriter out)
        {
            super(helper, "heartbeat always", com, out);
            setServerACM(1, -1, 3); // Enable server heartbeats.
        }

        public void runTestCase(RemoteObjectAdapterPrx adapter, TestIntfPrx proxy)
        {
            for(int i = 0; i < 10; i++)
            {
                proxy.ice_ping();
                try
                {
                    Thread.sleep(300);
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
    }

    static class HeartbeatManualTest extends TestCase
    {
        public HeartbeatManualTest(TestHelper helper, RemoteCommunicatorPrx com, java.io.PrintWriter out)
        {
            super(helper, "manual heartbeats", com, out);
            //
            // Disable heartbeats.
            //
            setClientACM(10, -1, 0);
            setServerACM(10, -1, 0);
        }

        public void runTestCase(RemoteObjectAdapterPrx adapter, TestIntfPrx proxy)
        {
            proxy.startHeartbeatCount();
            com.zeroc.Ice.Connection con = proxy.ice_getConnection();
            con.heartbeat();
            con.heartbeat();
            con.heartbeat();
            con.heartbeat();
            con.heartbeat();
            proxy.waitForHeartbeatCount(5);
        }
    }

    static class SetACMTest extends TestCase
    {
        public SetACMTest(TestHelper helper, RemoteCommunicatorPrx com, java.io.PrintWriter out)
        {
            super(helper, "setACM/getACM", com, out);
            setClientACM(15, 4, 0);
        }

        public void runTestCase(RemoteObjectAdapterPrx adapter, TestIntfPrx proxy)
        {
            com.zeroc.Ice.Connection con = proxy.ice_getConnection();

            try
            {
                con.setACM(java.util.OptionalInt.of(-19), null, null);
                test(false);
            }
            catch(IllegalArgumentException ex)
            {
            }

            com.zeroc.Ice.ACM acm = new com.zeroc.Ice.ACM();
            acm = con.getACM();
            test(acm.timeout == 15);
            test(acm.close == ACMClose.CloseOnIdleForceful);
            test(acm.heartbeat == ACMHeartbeat.HeartbeatOff);

            con.setACM(null, null, null);
            acm = con.getACM();
            test(acm.timeout == 15);
            test(acm.close == ACMClose.CloseOnIdleForceful);
            test(acm.heartbeat == ACMHeartbeat.HeartbeatOff);

            con.setACM(java.util.OptionalInt.of(1),
                       java.util.Optional.of(ACMClose.CloseOnInvocationAndIdle),
                       java.util.Optional.of(ACMHeartbeat.HeartbeatAlways));
            acm = con.getACM();
            test(acm.timeout == 1);
            test(acm.close == ACMClose.CloseOnInvocationAndIdle);
            test(acm.heartbeat == ACMHeartbeat.HeartbeatAlways);

            // Make sure the client sends few heartbeats to the server
            proxy.startHeartbeatCount();
            proxy.waitForHeartbeatCount(2);

            final CompletableFuture<Void> f1 = new CompletableFuture<>();
            con.setCloseCallback(c ->
                    {
                        f1.complete(null);
                    });

            con.close(com.zeroc.Ice.ConnectionClose.Gracefully);
            try
            {
                f1.get();
            }
            catch(Exception ex)
            {
                test(false);
            }

            try
            {
                con.throwException();
                test(false);
            }
            catch(com.zeroc.Ice.ConnectionManuallyClosedException ex)
            {
            }

            final CompletableFuture<Void> f2 = new CompletableFuture<>();
            con.setCloseCallback(c ->
                    {
                        f2.complete(null);
                    });
            try
            {
                f2.get();
            }
            catch(Exception ex)
            {
                test(false);
            }

            con.setHeartbeatCallback(c ->
                    {
                        test(false);
                    });
        }
    }

    public static void
    allTests(test.TestHelper helper)
    {
        com.zeroc.Ice.Communicator communicator = helper.communicator();
        PrintWriter out = helper.getWriter();

        String ref = "communicator:" + helper.getTestEndpoint();
        RemoteCommunicatorPrx com = RemoteCommunicatorPrx.uncheckedCast(communicator.stringToProxy(ref));

        java.util.List<TestCase> tests = new java.util.ArrayList<>();

        tests.add(new InvocationHeartbeatTest(helper, com, out));
        tests.add(new InvocationHeartbeatOnHoldTest(helper, com, out));
        tests.add(new InvocationNoHeartbeatTest(helper, com, out));
        tests.add(new InvocationHeartbeatCloseOnIdleTest(helper, com, out));

        tests.add(new CloseOnIdleTest(helper, com, out));
        tests.add(new CloseOnInvocationTest(helper, com, out));
        tests.add(new CloseOnIdleAndInvocationTest(helper, com, out));
        tests.add(new ForcefulCloseOnIdleAndInvocationTest(helper, com, out));

        tests.add(new HeartbeatOnIdleTest(helper, com, out));
        tests.add(new HeartbeatAlwaysTest(helper, com, out));
        tests.add(new HeartbeatManualTest(helper, com, out));
        tests.add(new SetACMTest(helper, com, out));

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
