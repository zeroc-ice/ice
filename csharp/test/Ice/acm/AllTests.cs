//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using Test;
using System.Globalization;
using System.IO;
using System.Threading;
using System.Threading.Tasks;

namespace Ice.acm
{
    class LoggerI : ILogger
    {
        public LoggerI(string name, TextWriter output)
        {
            _name = name;
            _output = output;
        }

        public void Start()
        {
            lock (this)
            {
                _started = true;
                Dump();
            }
        }

        public void Print(string msg)
        {
            lock (this)
            {
                _messages.Add(msg);
                if (_started)
                {
                    Dump();
                }
            }
        }

        public void Trace(string category, string message)
        {
            lock (this)
            {
                var s = new System.Text.StringBuilder(_name);
                s.Append(' ');
                s.Append(DateTime.Now.ToString(_date, CultureInfo.CurrentCulture));
                s.Append(' ');
                s.Append(DateTime.Now.ToString(_time, CultureInfo.CurrentCulture));
                s.Append(' ');
                s.Append("[");
                s.Append(category);
                s.Append("] ");
                s.Append(message);
                _messages.Add(s.ToString());
                if (_started)
                {
                    Dump();
                }
            }
        }

        public void Warning(string message)
        {
            lock (this)
            {
                var s = new System.Text.StringBuilder(_name);
                s.Append(' ');
                s.Append(DateTime.Now.ToString(_date, CultureInfo.CurrentCulture));
                s.Append(' ');
                s.Append(DateTime.Now.ToString(_time, CultureInfo.CurrentCulture));
                s.Append(" warning : ");
                s.Append(message);
                _messages.Add(s.ToString());
                if (_started)
                {
                    Dump();
                }
            }
        }

        public void Error(string message)
        {
            lock (this)
            {
                var s = new System.Text.StringBuilder(_name);
                s.Append(' ');
                s.Append(DateTime.Now.ToString(_date, CultureInfo.CurrentCulture));
                s.Append(' ');
                s.Append(DateTime.Now.ToString(_time, CultureInfo.CurrentCulture));
                s.Append(" error : ");
                s.Append(message);
                _messages.Add(s.ToString());
                if (_started)
                {
                    Dump();
                }
            }
        }

        public string GetPrefix() => "";

        public ILogger CloneWithPrefix(string prefix) => this;

        private void Dump()
        {
            foreach (string line in _messages)
            {
                _output.WriteLine(line);
            }
            _messages.Clear();
        }

        private readonly string _name;
        private bool _started;
        private readonly static string _date = "d";
        private readonly static string _time = "HH:mm:ss:fff";
        private readonly TextWriter _output;
        private readonly List<string> _messages = new List<string>();
    }

    abstract class TestCase
    {
        public TestCase(string name, Test.IRemoteCommunicatorPrx com, global::Test.TestHelper helper)
        {
            _name = name;
            _com = com;
            _output = helper.GetWriter();
            _logger = new LoggerI(_name, _output);
            _helper = helper;

            _clientACMTimeout = -1;
            _clientACMClose = -1;
            _clientACMHeartbeat = -1;

            _serverACMTimeout = -1;
            _serverACMClose = -1;
            _serverACMHeartbeat = -1;

            Heartbeat = 0;
            Closed = false;
        }

        public void Init()
        {
            _adapter = _com.createObjectAdapter(_serverACMTimeout, _serverACMClose, _serverACMHeartbeat)!;

            Dictionary<string, string> properties = _com.Communicator.GetProperties();
            properties["Ice.ACM.Timeout"] = "2";
            if (_clientACMTimeout >= 0)
            {
                properties["Ice.ACM.Client.Timeout"] = _clientACMTimeout.ToString();
            }

            if (_clientACMClose >= 0)
            {
                properties["Ice.ACM.Client.Close"] = _clientACMClose.ToString();
            }

            if (_clientACMHeartbeat >= 0)
            {
                properties["Ice.ACM.Client.Heartbeat"] = _clientACMHeartbeat.ToString();
            }
            _communicator = _helper.Initialize(properties);
            _thread = new Thread(Run);
        }

        public void Start() => _thread!.Start();

        public void Destroy()
        {
            _adapter!.deactivate();
            _communicator!.Destroy();
        }

        public void Join()
        {
            _output.Write("testing " + _name + "... ");
            _output.Flush();
            _logger.Start();
            _thread!.Join();
            if (_msg == null)
            {
                _output.WriteLine("ok");
            }
            else
            {
                _output.WriteLine("failed! " + _msg);
                throw new Exception();
            }
        }

        public void Run()
        {
            var proxy = Test.ITestIntfPrx.Parse(_adapter!.getTestIntf()!.ToString() ?? "", _communicator!);
            try
            {
                proxy.GetConnection().SetCloseCallback(_ =>
                {
                    lock (this)
                    {
                        Closed = true;
                        Monitor.Pulse(this);
                    }
                });

                proxy.GetConnection().SetHeartbeatCallback(_ =>
                {
                    lock (this)
                    {
                        ++Heartbeat;
                    }
                });

                RunTestCase(_adapter, proxy);
            }
            catch (Exception ex)
            {
                _msg = "unexpected exception:\n" + ex.ToString();
            }
        }

        public void WaitForClosed()
        {
            lock (this)
            {
                long now = IceInternal.Time.CurrentMonotonicTimeMillis();
                while (!Closed)
                {
                    Monitor.Wait(this, 30000);
                    if (IceInternal.Time.CurrentMonotonicTimeMillis() - now > 30000)
                    {
                        TestHelper.Assert(false); // Waited for more than 30s for close, something's wrong.
                        throw new Exception();
                    }
                }
            }
        }

        public abstract void RunTestCase(Test.IRemoteObjectAdapterPrx adapter, Test.ITestIntfPrx proxy);

        public void SetClientACM(int timeout, int close, int heartbeat)
        {
            _clientACMTimeout = timeout;
            _clientACMClose = close;
            _clientACMHeartbeat = heartbeat;
        }

        public void SetServerACM(int timeout, int close, int heartbeat)
        {
            _serverACMTimeout = timeout;
            _serverACMClose = close;
            _serverACMHeartbeat = heartbeat;
        }

        private readonly string _name;
        private readonly Test.IRemoteCommunicatorPrx _com;
        private string? _msg;
        private readonly LoggerI _logger;
        private readonly global::Test.TestHelper _helper;
        private readonly TextWriter _output;
        private Thread? _thread;

        private Communicator? _communicator;
        private Test.IRemoteObjectAdapterPrx? _adapter;

        private int _clientACMTimeout;
        private int _clientACMClose;
        private int _clientACMHeartbeat;
        private int _serverACMTimeout;
        private int _serverACMClose;
        private int _serverACMHeartbeat;

        protected int Heartbeat;
        protected bool Closed;
    }

    public class AllTests
    {
        class InvocationHeartbeatTest : TestCase
        {
            public InvocationHeartbeatTest(Test.IRemoteCommunicatorPrx com, global::Test.TestHelper helper) :
                base("invocation heartbeat", com, helper) => SetServerACM(1, -1, -1); // Faster ACM to make sure we receive enough ACM heartbeats

            public override void RunTestCase(Test.IRemoteObjectAdapterPrx adapter, Test.ITestIntfPrx proxy)
            {
                proxy.sleep(4);

                lock (this)
                {
                    TestHelper.Assert(Heartbeat >= 4);
                }
            }
        }

        class InvocationNoHeartbeatTest : TestCase
        {
            // Disable heartbeat on invocations
            public InvocationNoHeartbeatTest(Test.IRemoteCommunicatorPrx com, global::Test.TestHelper helper) :
                base("invocation with no heartbeat", com, helper) => SetServerACM(2, 2, 0);

            public override void RunTestCase(Test.IRemoteObjectAdapterPrx adapter, Test.ITestIntfPrx proxy)
            {
                try
                {
                    // Heartbeats are disabled on the server, the
                    // invocation should fail since heartbeats are
                    // expected.
                    proxy.sleep(10);
                    TestHelper.Assert(false);
                }
                catch (ConnectionTimeoutException)
                {
                    proxy.interruptSleep();

                    WaitForClosed();
                    lock (this)
                    {
                        TestHelper.Assert(Heartbeat == 0);
                    }
                }
            }
        }

        class InvocationHeartbeatCloseOnIdleTest : TestCase
        {
            public InvocationHeartbeatCloseOnIdleTest(Test.IRemoteCommunicatorPrx com, global::Test.TestHelper helper) :
                base("invocation with no heartbeat and close on idle", com, helper)
            {
                SetClientACM(1, 1, 0); // Only close on idle.
                SetServerACM(1, 2, 0); // Disable heartbeat on invocations
            }

            public override void RunTestCase(Test.IRemoteObjectAdapterPrx adapter, Test.ITestIntfPrx proxy)
            {
                // No close on invocation, the call should succeed this
                // time.
                proxy.sleep(3);

                lock (this)
                {
                    TestHelper.Assert(Heartbeat == 0);
                    TestHelper.Assert(!Closed);
                }
            }
        }

        class CloseOnIdleTest : TestCase
        {
            public CloseOnIdleTest(Test.IRemoteCommunicatorPrx com, global::Test.TestHelper helper) :
                base("close on idle", com, helper) => SetClientACM(1, 1, 0); // Only close on idle

            public override void RunTestCase(Test.IRemoteObjectAdapterPrx adapter, Test.ITestIntfPrx proxy)
            {
                Connection connection = proxy.GetConnection();
                WaitForClosed();
                lock (this)
                {
                    TestHelper.Assert(Heartbeat == 0);
                    try
                    {
                        connection.ThrowException();
                    }
                    catch (Exception ex)
                    {
                        TestHelper.Assert(ex is ConnectionIdleException);;
                    }
                }
            }
        }

        class CloseOnInvocationTest : TestCase
        {
            public CloseOnInvocationTest(Test.IRemoteCommunicatorPrx com, global::Test.TestHelper helper) :
                base("close on invocation", com, helper) => SetClientACM(1, 2, 0); // Only close on invocation

            public override void RunTestCase(Test.IRemoteObjectAdapterPrx adapter, Test.ITestIntfPrx proxy)
            {
                Thread.Sleep(3000); // Idle for 3 seconds

                lock (this)
                {
                    TestHelper.Assert(Heartbeat == 0);
                    TestHelper.Assert(!Closed);
                }
            }
        }

        class CloseOnIdleAndInvocationTest : TestCase
        {
            // Only close on idle and invocation
            public CloseOnIdleAndInvocationTest(Test.IRemoteCommunicatorPrx com, global::Test.TestHelper helper) :
                base("close on idle and invocation", com, helper) => SetClientACM(1, 3, 0);
            public override void RunTestCase(Test.IRemoteObjectAdapterPrx adapter, Test.ITestIntfPrx proxy)
            {
                Connection connection = proxy.GetConnection();
                WaitForClosed();
                lock (this)
                {
                    TestHelper.Assert(Heartbeat == 0);
                }
                try
                {
                    connection.ThrowException();
                }
                catch (Exception ex)
                {
                    TestHelper.Assert(ex is ConnectionIdleException);;
                }
            }
        }

        class ForcefulCloseOnIdleAndInvocationTest : TestCase
        {
            // Only close on idle and invocation
            public ForcefulCloseOnIdleAndInvocationTest(Test.IRemoteCommunicatorPrx com, global::Test.TestHelper helper) :
                base("forceful close on idle and invocation", com, helper) => SetClientACM(1, 4, 0);
            public override void RunTestCase(Test.IRemoteObjectAdapterPrx adapter, Test.ITestIntfPrx proxy)
            {
                Connection connection = proxy.GetConnection();
                WaitForClosed();
                lock (this)
                {
                    TestHelper.Assert(Heartbeat == 0);
                }
                try
                {
                    connection.ThrowException();
                }
                catch (Exception ex)
                {
                    TestHelper.Assert(ex is ConnectionTimeoutException);;
                }
            }
        }

        class HeartbeatOnIdleTest : TestCase
        {
            public HeartbeatOnIdleTest(Test.IRemoteCommunicatorPrx com, global::Test.TestHelper helper) :
                base("heartbeat on idle", com, helper) => SetServerACM(1, -1, 2); // Enable server heartbeats.

            public override void RunTestCase(Test.IRemoteObjectAdapterPrx adapter, Test.ITestIntfPrx proxy)
            {
                Thread.Sleep(3000);

                lock (this)
                {
                    TestHelper.Assert(Heartbeat >= 3);
                }
            }
        }

        class HeartbeatAlwaysTest : TestCase
        {
            // Enable server heartbeats.
            public HeartbeatAlwaysTest(Test.IRemoteCommunicatorPrx com, global::Test.TestHelper helper) :
                base("heartbeat always", com, helper) => SetServerACM(1, -1, 3);

            public override void RunTestCase(Test.IRemoteObjectAdapterPrx adapter, Test.ITestIntfPrx proxy)
            {
                for (int i = 0; i < 10; i++)
                {
                    proxy.IcePing();
                    Thread.Sleep(300);
                }

                lock (this)
                {
                    TestHelper.Assert(Heartbeat >= 3);
                }
            }
        }

        class HeartbeatManualTest : TestCase
        {
            public HeartbeatManualTest(Test.IRemoteCommunicatorPrx com, global::Test.TestHelper helper) :
                base("manual heartbeats", com, helper)
            {
                //
                // Disable heartbeats.
                //
                SetClientACM(10, -1, 0);
                SetServerACM(10, -1, 0);
            }

            public override void RunTestCase(Test.IRemoteObjectAdapterPrx adapter, Test.ITestIntfPrx proxy)
            {
                proxy.startHeartbeatCount();
                Connection con = proxy.GetConnection();
                con.Heartbeat();
                con.Heartbeat();
                con.Heartbeat();
                con.Heartbeat();
                con.Heartbeat();
                proxy.waitForHeartbeatCount(5);
            }
        }

        class SetACMTest : TestCase
        {
            public SetACMTest(Test.IRemoteCommunicatorPrx com, global::Test.TestHelper helper) :
                base("setACM/getACM", com, helper) => SetClientACM(15, 4, 0);

            public override void RunTestCase(Test.IRemoteObjectAdapterPrx adapter, Test.ITestIntfPrx proxy)
            {
                Connection? con = proxy.GetCachedConnection();
                TestHelper.Assert(con != null);
                try
                {
                    con.SetACM(-19, null, null);
                    TestHelper.Assert(false);
                }
                catch (ArgumentException)
                {
                }

                Ice.ACM acm = con.GetACM();
                TestHelper.Assert(acm.Timeout == 15);
                TestHelper.Assert(acm.Close == Ice.ACMClose.CloseOnIdleForceful);
                TestHelper.Assert(acm.Heartbeat == Ice.ACMHeartbeat.HeartbeatOff);

                con.SetACM(null, null, null);
                acm = con.GetACM();
                TestHelper.Assert(acm.Timeout == 15);
                TestHelper.Assert(acm.Close == Ice.ACMClose.CloseOnIdleForceful);
                TestHelper.Assert(acm.Heartbeat == Ice.ACMHeartbeat.HeartbeatOff);

                con.SetACM(1, Ice.ACMClose.CloseOnInvocationAndIdle, Ice.ACMHeartbeat.HeartbeatAlways);
                acm = con.GetACM();
                TestHelper.Assert(acm.Timeout == 1);
                TestHelper.Assert(acm.Close == Ice.ACMClose.CloseOnInvocationAndIdle);
                TestHelper.Assert(acm.Heartbeat == Ice.ACMHeartbeat.HeartbeatAlways);

                proxy.startHeartbeatCount();
                proxy.waitForHeartbeatCount(2);

                var t1 = new TaskCompletionSource<object?>();
                con.SetCloseCallback(_ => t1.SetResult(null));

                con.Close(ConnectionClose.Gracefully);
                TestHelper.Assert(t1.Task.Result == null);

                try
                {
                    con.ThrowException();
                    TestHelper.Assert(false);
                }
                catch (ConnectionClosedLocallyException)
                {
                }

                var t2 = new TaskCompletionSource<object?>();
                con.SetCloseCallback(_ => t2.SetResult(null));
                TestHelper.Assert(t2.Task.Result == null);

                con.SetHeartbeatCallback(_ => TestHelper.Assert(false));
            }
        }

        public static void allTests(global::Test.TestHelper helper)
        {
            Communicator? communicator = helper.Communicator();
            TestHelper.Assert(communicator != null);
            var com = Test.IRemoteCommunicatorPrx.Parse($"communicator:{helper.GetTestEndpoint(0)}", communicator);

            TextWriter output = helper.GetWriter();

            var tests = new List<TestCase>
            {
                new InvocationHeartbeatTest(com, helper),
                new InvocationNoHeartbeatTest(com, helper),
                new InvocationHeartbeatCloseOnIdleTest(com, helper),

                new CloseOnIdleTest(com, helper),
                new CloseOnInvocationTest(com, helper),
                new CloseOnIdleAndInvocationTest(com, helper),
                new ForcefulCloseOnIdleAndInvocationTest(com, helper),

                new HeartbeatOnIdleTest(com, helper),
                new HeartbeatAlwaysTest(com, helper),
                new HeartbeatManualTest(com, helper),
                new SetACMTest(com, helper)
            };

            foreach (TestCase test in tests)
            {
                test.Init();
            }
            foreach (TestCase test in tests)
            {
                test.Start();
            }
            foreach (TestCase test in tests)
            {
                test.Join();
            }
            foreach (TestCase test in tests)
            {
                test.Destroy();
            }

            output.Write("shutting down... ");
            output.Flush();
            com.shutdown();
            output.WriteLine("ok");
        }
    }
}
