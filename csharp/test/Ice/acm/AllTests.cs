// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading;
using Test;
#if SILVERLIGHT
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Ink;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;
#endif

class LoggerI : Ice.Logger
{
    public void start()
    {
        lock(this)
        {
            _started = true;
            dump();
        }
    }

    public void print(string msg)
    {
        lock(this)
        {
            _messages.Add(msg);
            if(_started)
            {
                dump();
            }
        }
    }

    public void trace(string category, string message)
    {
        lock(this)
        {
            _messages.Add("[" + category + "] " + message);
            if(_started)
            {
                dump();
            }
        }
    }

    public void warning(string message)
    {
        lock(this)
        {
            _messages.Add("warning: " + message);
            if(_started)
            {
                dump();
            }
        }
    }

    public void error(string message)
    {
        lock(this)
        {
            _messages.Add("error: " + message);
            if(_started)
            {
                dump();
            }
        }
    }

    public string getPrefix()
    {
        return "";
    }

    public Ice.Logger cloneWithPrefix(string prefix)
    {
        return this;
    }

    private void dump()
    {
        foreach(string line in _messages)
        {
            System.Console.WriteLine(line);
        }
        _messages.Clear();
    }

    private bool _started;
    private List<string> _messages = new List<string>();
};

abstract class TestCase : Ice.ConnectionCallback
{
    public TestCase(string name, RemoteCommunicatorPrx com)
    {
        _name = name;
        _com = com;
        _logger = new LoggerI();

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
        initData.properties = _com.ice_getCommunicator().getProperties().ice_clone_();
        initData.logger = _logger;
        initData.properties.setProperty("Ice.ACM.Timeout", "1");
        if(_clientACMTimeout >= 0)
        {
            initData.properties.setProperty("Ice.ACM.Client.Timeout", _clientACMTimeout.ToString());
        }
        if(_clientACMClose >= 0)
        {
            initData.properties.setProperty("Ice.ACM.Client.Close", _clientACMClose.ToString());
        }
        if(_clientACMHeartbeat >= 0)
        {
            initData.properties.setProperty("Ice.ACM.Client.Heartbeat", _clientACMHeartbeat.ToString());
        }
        //initData.properties.setProperty("Ice.Trace.Protocol", "2");
        //initData.properties.setProperty("Ice.Trace.Network", "2");
        _communicator = Ice.Util.initialize(initData);
        _thread = new Thread(this.run);
    }

    public void start()
    {
        _thread.Start();
    }

    public void destroy()
    {
        _adapter.deactivate();
        _communicator.destroy();
    }

    public void join()
    {
        System.Console.Out.Write("testing " + _name + "... ");
        System.Console.Out.Flush();
        _logger.start();
        _thread.Join();
        if(_msg == null)
        {
            System.Console.Out.WriteLine("ok");
        }
        else
        {
            System.Console.Out.WriteLine("failed! " + _msg);
            throw new System.Exception();
        }
    }

    public void run()
    {
        TestIntfPrx proxy = TestIntfPrxHelper.uncheckedCast(_communicator.stringToProxy(
                                                                _adapter.getTestIntf().ToString()));
        try
        {
            proxy.ice_getConnection().setCallback(this);
            runTestCase(_adapter, proxy);
        }
        catch(Exception ex)
        {
            _msg = "unexpected exception:\n" + ex.ToString();
        }
    }

    public void heartbeat(Ice.Connection con)
    {
        lock(this)
        {
            ++_heartbeat;
        }
    }

    public void closed(Ice.Connection con)
    {
        lock(this)
        {
            _closed = true;
            Monitor.Pulse(this);
        }
    }

    public void waitForClosed()
    {
        lock(this)
        {
            long now = IceInternal.Time.currentMonotonicTimeMillis();
            while(!_closed)
            {
                Monitor.Wait(this, 1000);
                if(IceInternal.Time.currentMonotonicTimeMillis() - now > 1000)
                {
                    System.Diagnostics.Debug.Assert(false); // Waited for more than 1s for close, something's wrong.
                    throw new System.Exception();
                }
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

    private string _name;
    private RemoteCommunicatorPrx _com;
    private string _msg;
    private LoggerI _logger;
    private System.Threading.Thread _thread;

    private Ice.Communicator _communicator;
    private RemoteObjectAdapterPrx _adapter;

    private int _clientACMTimeout;
    private int _clientACMClose;
    private int _clientACMHeartbeat;
    private int _serverACMTimeout;
    private int _serverACMClose;
    private int _serverACMHeartbeat;

    protected int _heartbeat;
    protected bool _closed;
};

public class AllTests : TestCommon.TestApp
{
    class InvocationHeartbeatTest : TestCase
    {
        public InvocationHeartbeatTest(RemoteCommunicatorPrx com) : base("invocation heartbeat", com)
        {
        }

        public override void runTestCase(RemoteObjectAdapterPrx adapter, TestIntfPrx proxy)
        {
            proxy.sleep(2);

            lock(this)
            {
                test(_heartbeat >= 2);
            }
        }
    };

    class InvocationHeartbeatOnHoldTest : TestCase
    {
        public InvocationHeartbeatOnHoldTest(RemoteCommunicatorPrx com) :
            base("invocation with heartbeat on hold", com)
        {
                // Use default ACM configuration.
        }

        public override void runTestCase(RemoteObjectAdapterPrx adapter, TestIntfPrx proxy)
        {
            try
            {
                // When the OA is put on hold, connections shouldn't
                // send heartbeats, the invocation should therefore
                // fail.
                proxy.sleepAndHold(10);
                test(false);
            }
            catch(Ice.ConnectionTimeoutException)
            {
                adapter.activate();
                proxy.interruptSleep();

                waitForClosed();
            }
        }
    };

    class InvocationNoHeartbeatTest : TestCase
    {
        public InvocationNoHeartbeatTest(RemoteCommunicatorPrx com) : base("invocation with no heartbeat", com)
        {
            setServerACM(1, 2, 0); // Disable heartbeat on invocations
        }

        public override void runTestCase(RemoteObjectAdapterPrx adapter, TestIntfPrx proxy)
        {
            try
            {
                // Heartbeats are disabled on the server, the
                // invocation should fail since heartbeats are
                // expected.
                proxy.sleep(10);
                test(false);
            }
            catch(Ice.ConnectionTimeoutException)
            {
                proxy.interruptSleep();

                waitForClosed();
                lock(this)
                {
                    test(_heartbeat == 0);
                }
            }
        }
    };

    class InvocationHeartbeatCloseOnIdleTest : TestCase
    {
        public InvocationHeartbeatCloseOnIdleTest(RemoteCommunicatorPrx com) :
            base("invocation with no heartbeat and close on idle", com)
        {
            setClientACM(1, 1, 0); // Only close on idle.
            setServerACM(1, 2, 0); // Disable heartbeat on invocations
        }

        public override void runTestCase(RemoteObjectAdapterPrx adapter, TestIntfPrx proxy)
        {
            // No close on invocation, the call should succeed this
            // time.
            proxy.sleep(2);

            lock(this)
            {
                test(_heartbeat == 0);
                test(!_closed);
            }
        }
    };

    class CloseOnIdleTest : TestCase
    {
        public CloseOnIdleTest(RemoteCommunicatorPrx com) : base("close on idle", com)
        {
            setClientACM(1, 1, 0); // Only close on idle
        }

        public override void runTestCase(RemoteObjectAdapterPrx adapter, TestIntfPrx proxy)
        {
            Thread.Sleep(1500); // Idle for 1.5 second

            waitForClosed();
            lock(this)
            {
                test(_heartbeat == 0);
            }
        }
    };

    class CloseOnInvocationTest : TestCase
    {
        public CloseOnInvocationTest(RemoteCommunicatorPrx com) : base("close on invocation", com)
        {
            setClientACM(1, 2, 0); // Only close on invocation
        }

        public override void runTestCase(RemoteObjectAdapterPrx adapter, TestIntfPrx proxy)
        {
            Thread.Sleep(1500); // Idle for 1.5 second

            lock(this)
            {
                test(_heartbeat == 0);
                test(!_closed);
            }
        }
    };

    class CloseOnIdleAndInvocationTest : TestCase
    {
        public CloseOnIdleAndInvocationTest(RemoteCommunicatorPrx com) : base("close on idle and invocation", com)
        {
            setClientACM(1, 3, 0); // Only close on idle and invocation
        }

        public override void runTestCase(RemoteObjectAdapterPrx adapter, TestIntfPrx proxy)
        {
            //
            // Put the adapter on hold. The server will not respond to
            // the graceful close. This allows to test whether or not
            // the close is graceful or forceful.
            //
            adapter.hold();
            Thread.Sleep(1500); // Idle for 1.5 second

            lock(this)
            {
                test(_heartbeat == 0);
                test(!_closed); // Not closed yet because of graceful close.
            }

            adapter.activate();
            Thread.Sleep(500);

            waitForClosed();
        }
    };

    class ForcefulCloseOnIdleAndInvocationTest : TestCase
    {
        public ForcefulCloseOnIdleAndInvocationTest(RemoteCommunicatorPrx com) :
            base("forceful close on idle and invocation", com)
        {
            setClientACM(1, 4, 0); // Only close on idle and invocation
        }

        public override void runTestCase(RemoteObjectAdapterPrx adapter, TestIntfPrx proxy)
        {
            adapter.hold();
            Thread.Sleep(1500); // Idle for 1.5 second

            waitForClosed();
            lock(this)
            {
                test(_heartbeat == 0);
            }
        }
    };

    class HeartbeatOnIdleTest : TestCase
    {
        public HeartbeatOnIdleTest(RemoteCommunicatorPrx com) : base("heartbeat on idle", com)
        {
            setServerACM(1, -1, 2); // Enable server heartbeats.
        }

        public override void runTestCase(RemoteObjectAdapterPrx adapter, TestIntfPrx proxy)
        {
            Thread.Sleep(2000);

            lock(this)
            {
                test(_heartbeat >= 3);
            }
        }
    };

    class HeartbeatAlwaysTest : TestCase
    {
        public HeartbeatAlwaysTest(RemoteCommunicatorPrx com) : base("heartbeat always", com)
        {
            setServerACM(1, -1, 3); // Enable server heartbeats.
        }

        public override void runTestCase(RemoteObjectAdapterPrx adapter, TestIntfPrx proxy)
        {
            for(int i = 0; i < 12; i++)
            {
                proxy.ice_ping();
                Thread.Sleep(200);
            }

            lock(this)
            {
                test(_heartbeat >= 3);
            }
        }
    };

    class SetACMTest : TestCase
    {
        public SetACMTest(RemoteCommunicatorPrx com) : base("setACM/getACM", com)
        {
            setClientACM(15, 4, 0);
        }

        public override void runTestCase(RemoteObjectAdapterPrx adapter, TestIntfPrx proxy)
        {
            Ice.ACM acm;
            acm = proxy.ice_getCachedConnection().getACM();
            test(acm.timeout == 15);
            test(acm.close == Ice.ACMClose.CloseOnIdleForceful);
            test(acm.heartbeat == Ice.ACMHeartbeat.HeartbeatOff);

            proxy.ice_getCachedConnection().setACM(Ice.Util.None, Ice.Util.None, Ice.Util.None);
            acm = proxy.ice_getCachedConnection().getACM();
            test(acm.timeout == 15);
            test(acm.close == Ice.ACMClose.CloseOnIdleForceful);
            test(acm.heartbeat == Ice.ACMHeartbeat.HeartbeatOff);

            proxy.ice_getCachedConnection().setACM(1,
                                                   Ice.ACMClose.CloseOnInvocationAndIdle,
                                                   Ice.ACMHeartbeat.HeartbeatAlways);
            acm = proxy.ice_getCachedConnection().getACM();
            test(acm.timeout == 1);
            test(acm.close == Ice.ACMClose.CloseOnInvocationAndIdle);
            test(acm.heartbeat == Ice.ACMHeartbeat.HeartbeatAlways);

            proxy.waitForHeartbeat(2);
        }
    };


#if SILVERLIGHT
    override
    public void run(Ice.Communicator communicator)
#else
    public static void allTests(Ice.Communicator communicator)
#endif
    {
        string @ref = "communicator:default -p 12010";
        RemoteCommunicatorPrx com = RemoteCommunicatorPrxHelper.uncheckedCast(communicator.stringToProxy(@ref));

        List<TestCase> tests = new List<TestCase>();

        tests.Add(new InvocationHeartbeatTest(com));
        tests.Add(new InvocationHeartbeatOnHoldTest(com));
        tests.Add(new InvocationNoHeartbeatTest(com));
        tests.Add(new InvocationHeartbeatCloseOnIdleTest(com));

        tests.Add(new CloseOnIdleTest(com));
        tests.Add(new CloseOnInvocationTest(com));
        tests.Add(new CloseOnIdleAndInvocationTest(com));
        tests.Add(new ForcefulCloseOnIdleAndInvocationTest(com));

        tests.Add(new HeartbeatOnIdleTest(com));
        tests.Add(new HeartbeatAlwaysTest(com));
        tests.Add(new SetACMTest(com));

        foreach(TestCase test in tests)
        {
            test.init();
        }
        foreach(TestCase test in tests)
        {
            test.start();
        }
        foreach(TestCase test in tests)
        {
            test.join();
        }
        foreach(TestCase test in tests)
        {
            test.destroy();
        }

        System.Console.Out.Write("shutting down... ");
        System.Console.Out.Flush();
        com.shutdown();
        System.Console.WriteLine("ok");
    }
}
