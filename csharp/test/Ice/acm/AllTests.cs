//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Threading;
using System.Threading.Tasks;
using Test;

namespace ZeroC.Ice.Test.ACM
{
    public class Logger : ILogger
    {
        private readonly object _mutex = new object();
        public Logger(string name, TextWriter output)
        {
            _name = name;
            _output = output;
        }

        public void Start()
        {
            lock (_mutex)
            {
                _started = true;
                Dump();
            }
        }

        public void Print(string msg)
        {
            lock (_mutex)
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
            lock (_mutex)
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
            lock (_mutex)
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
            lock (_mutex)
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

        public string Prefix => "";

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
        private const string _date = "d";
        private const string _time = "HH:mm:ss:fff";
        private readonly TextWriter _output;
        private readonly List<string> _messages = new List<string>();
    }

    public abstract class TestCase
    {
        protected readonly object _mutex = new object();

        public TestCase(string name, IRemoteCommunicatorPrx com, TestHelper helper)
        {
            _name = name;
            _com = com;
            _output = helper.GetWriter();
            _logger = new Logger(_name, _output);
            _helper = helper;

            _clientAcmTimeout = -1;
            _clientAcmClose = null;
            _clientAcmHeartbeat = null;

            _serverAcmTimeout = -1;
            _serverAcmClose = null;
            _serverAcmHeartbeat = null;

            Heartbeat = 0;
            Closed = false;
        }

        public void Init()
        {
            _adapter = _com.CreateObjectAdapter(_serverAcmTimeout,
                                                _serverAcmClose?.ToString(),
                                                _serverAcmHeartbeat?.ToString());

            Dictionary<string, string> properties = _com.Communicator.GetProperties();
            properties["Ice.ACM.Timeout"] = "2s";
            if (_clientAcmTimeout >= 0)
            {
                properties["Ice.ACM.Client.Timeout"] = $"{_clientAcmTimeout}s";
            }

            if (_clientAcmClose >= 0)
            {
                properties["Ice.ACM.Client.Close"] = ((AcmClose)_clientAcmClose).ToString();
            }

            if (_clientAcmHeartbeat >= 0)
            {
                properties["Ice.ACM.Client.Heartbeat"] = ((AcmHeartbeat)_clientAcmHeartbeat).ToString();
            }
            _communicator = _helper.Initialize(properties);
            _thread = new Thread(Run);
        }

        public void Start() => _thread!.Start();

        public void Destroy()
        {
            _adapter!.Deactivate();
            _communicator!.Dispose();
        }

        public void Join()
        {
            _output.Write($"testing {_name}... ");
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
            var proxy = ITestIntfPrx.Parse(_adapter!.GetTestIntf()!.ToString() ?? "", _communicator!);
            try
            {
                proxy.GetConnection()!.Closed += (sender, args) =>
                    {
                        lock (_mutex)
                        {
                            Closed = true;
                            Monitor.Pulse(_mutex);
                        }
                    };

                proxy.GetConnection()!.HeartbeatReceived += (sender, args) =>
                    {
                        lock (_mutex)
                        {
                            ++Heartbeat;
                        }
                    };

                RunTestCase(_adapter, proxy);
            }
            catch (Exception ex)
            {
                _msg = "unexpected exception:\n" + ex.ToString();
            }
        }

        public void WaitForClosed()
        {
            lock (_mutex)
            {
                TimeSpan now = Time.Elapsed;
                while (!Closed)
                {
                    Monitor.Wait(_mutex, TimeSpan.FromSeconds(30));
                    if (Time.Elapsed - now > TimeSpan.FromSeconds(30))
                    {
                        TestHelper.Assert(false); // Waited for more than 30s for close, something's wrong.
                        throw new Exception();
                    }
                }
            }
        }

        public abstract void RunTestCase(IRemoteObjectAdapterPrx adapter, ITestIntfPrx proxy);

        public void SetClientAcm(int timeout, AcmClose? close, AcmHeartbeat? heartbeat)
        {
            _clientAcmTimeout = timeout;
            _clientAcmClose = close;
            _clientAcmHeartbeat = heartbeat;
        }

        public void SetServerAcm(int timeout, AcmClose? close, AcmHeartbeat? heartbeat)
        {
            _serverAcmTimeout = timeout;
            _serverAcmClose = close;
            _serverAcmHeartbeat = heartbeat;
        }

        private readonly string _name;
        private readonly IRemoteCommunicatorPrx _com;
        private string? _msg;
        private readonly Logger _logger;
        private readonly TestHelper _helper;
        private readonly TextWriter _output;
        private Thread? _thread;

        private Communicator? _communicator;
        private IRemoteObjectAdapterPrx? _adapter;

        private int _clientAcmTimeout;
        private AcmClose? _clientAcmClose;
        private AcmHeartbeat? _clientAcmHeartbeat;
        private int _serverAcmTimeout;
        private AcmClose? _serverAcmClose;
        private AcmHeartbeat? _serverAcmHeartbeat;

        protected int Heartbeat;
        protected bool Closed;
    }

    public class AllTests
    {
        public class InvocationHeartbeatTest : TestCase
        {
            // Faster ACM to make sure we receive enough ACM heartbeats
            public InvocationHeartbeatTest(IRemoteCommunicatorPrx com, TestHelper helper) :
                base("invocation heartbeat", com, helper) => SetServerAcm(1, null, null);
            public override void RunTestCase(IRemoteObjectAdapterPrx adapter, ITestIntfPrx proxy)
            {
                proxy.Sleep(4);

                lock (_mutex)
                {
                    TestHelper.Assert(Heartbeat >= 4);
                }
            }
        }

        public class InvocationNoHeartbeatTest : TestCase
        {
            // Disable heartbeat on invocations
            public InvocationNoHeartbeatTest(IRemoteCommunicatorPrx com, TestHelper helper) :
                base("invocation with no heartbeat", com, helper) =>
                SetServerAcm(2, AcmClose.OnInvocation, AcmHeartbeat.Off);

            public override void RunTestCase(IRemoteObjectAdapterPrx adapter, ITestIntfPrx proxy)
            {
                try
                {
                    // Heartbeats are disabled on the server, the invocation should fail since heartbeats are expected.
                    proxy.Sleep(10);
                    TestHelper.Assert(false);
                }
                catch (ConnectionTimeoutException)
                {
                    proxy.InterruptSleep();

                    WaitForClosed();
                    lock (_mutex)
                    {
                        TestHelper.Assert(Heartbeat == 0);
                    }
                }
            }
        }

        public class InvocationHeartbeatCloseOnIdleTest : TestCase
        {
            public InvocationHeartbeatCloseOnIdleTest(IRemoteCommunicatorPrx com, TestHelper helper) :
                base("invocation with no heartbeat and close on idle", com, helper)
            {
                SetClientAcm(1, AcmClose.OnIdle, AcmHeartbeat.Off); // Only close on idle.
                SetServerAcm(1, AcmClose.OnInvocation, AcmHeartbeat.Off); // Disable heartbeat on invocations
            }

            public override void RunTestCase(IRemoteObjectAdapterPrx adapter, ITestIntfPrx proxy)
            {
                // No close on invocation, the call should succeed this time.
                proxy.Sleep(3);

                lock (_mutex)
                {
                    TestHelper.Assert(Heartbeat == 0);
                    TestHelper.Assert(!Closed);
                }
            }
        }

        public class CloseOnIdleTest : TestCase
        {
            // Only close on idle
            public CloseOnIdleTest(IRemoteCommunicatorPrx com, TestHelper helper) :
                base("close on idle", com, helper) => SetClientAcm(1, AcmClose.OnIdle, AcmHeartbeat.Off);

            public override void RunTestCase(IRemoteObjectAdapterPrx adapter, ITestIntfPrx proxy)
            {
                Connection connection = proxy.GetConnection()!;
                WaitForClosed();
                lock (_mutex)
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

        public class CloseOnInvocationTest : TestCase
        {
            // Only close on invocation
            public CloseOnInvocationTest(IRemoteCommunicatorPrx com, TestHelper helper) :
                base("close on invocation", com, helper) => SetClientAcm(1, AcmClose.OnInvocation, AcmHeartbeat.Off);

            public override void RunTestCase(IRemoteObjectAdapterPrx adapter, ITestIntfPrx proxy)
            {
                Thread.Sleep(3000); // Idle for 3 seconds

                lock (_mutex)
                {
                    TestHelper.Assert(Heartbeat == 0);
                    TestHelper.Assert(!Closed);
                }
            }
        }

        public class CloseOnIdleAndInvocationTest : TestCase
        {
            // Only close on idle and invocation
            public CloseOnIdleAndInvocationTest(IRemoteCommunicatorPrx com, TestHelper helper) :
                base("close on idle and invocation", com, helper) =>
                SetClientAcm(1, AcmClose.OnInvocationAndIdle, AcmHeartbeat.Off);
            public override void RunTestCase(IRemoteObjectAdapterPrx adapter, ITestIntfPrx proxy)
            {
                Connection connection = proxy.GetConnection()!;
                WaitForClosed();
                lock (_mutex)
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

        public class ForcefulCloseOnIdleAndInvocationTest : TestCase
        {
            // Only close on idle and invocation
            public ForcefulCloseOnIdleAndInvocationTest(IRemoteCommunicatorPrx com, TestHelper helper) :
                base("forceful close on idle and invocation", com, helper) =>
                SetClientAcm(1, AcmClose.OnIdleForceful, AcmHeartbeat.Off);
            public override void RunTestCase(IRemoteObjectAdapterPrx adapter, ITestIntfPrx proxy)
            {
                Connection connection = proxy.GetConnection()!;
                WaitForClosed();
                lock (_mutex)
                {
                    TestHelper.Assert(Heartbeat == 0);
                }
                try
                {
                    connection.ThrowException();
                }
                catch (Exception ex)
                {
                    TestHelper.Assert(ex is ConnectionTimeoutException);
                }
            }
        }

        public class HeartbeatOnIdleTest : TestCase
        {
            // Enable server heartbeats.
            public HeartbeatOnIdleTest(IRemoteCommunicatorPrx com, TestHelper helper) :
                base("heartbeat on idle", com, helper) => SetServerAcm(1, null, AcmHeartbeat.OnIdle);

            public override void RunTestCase(IRemoteObjectAdapterPrx adapter, ITestIntfPrx proxy)
            {
                Thread.Sleep(3000);

                lock (_mutex)
                {
                    TestHelper.Assert(Heartbeat >= 3);
                }
            }
        }

        public class HeartbeatAlwaysTest : TestCase
        {
            // Enable server heartbeats.
            public HeartbeatAlwaysTest(IRemoteCommunicatorPrx com, TestHelper helper) :
                base("heartbeat always", com, helper) => SetServerAcm(1, null, AcmHeartbeat.Always);

            public override void RunTestCase(IRemoteObjectAdapterPrx adapter, ITestIntfPrx proxy)
            {
                for (int i = 0; i < 10; i++)
                {
                    proxy.IcePing();
                    Thread.Sleep(300);
                }

                lock (_mutex)
                {
                    TestHelper.Assert(Heartbeat >= 3);
                }
            }
        }

        public class HeartbeatManualTest : TestCase
        {
            public HeartbeatManualTest(IRemoteCommunicatorPrx com, TestHelper helper) :
                base("manual heartbeats", com, helper)
            {
                // Disable heartbeats.
                SetClientAcm(10, null, AcmHeartbeat.Off);
                SetServerAcm(10, null, AcmHeartbeat.Off);
            }

            public override void RunTestCase(IRemoteObjectAdapterPrx adapter, ITestIntfPrx proxy)
            {
                proxy.StartHeartbeatCount();
                Connection con = proxy.GetConnection()!;
                con.Heartbeat();
                con.Heartbeat();
                con.Heartbeat();
                con.Heartbeat();
                con.Heartbeat();
                proxy.WaitForHeartbeatCount(5);
            }
        }

        public class SetAcmTest : TestCase
        {
            public SetAcmTest(IRemoteCommunicatorPrx com, TestHelper helper) :
                base("setACM/getACM", com, helper) => SetClientAcm(15, AcmClose.OnIdleForceful, AcmHeartbeat.Off);

            public override void RunTestCase(IRemoteObjectAdapterPrx adapter, ITestIntfPrx proxy)
            {
                Connection? con = proxy.GetCachedConnection();
                TestHelper.Assert(con != null);

                Acm acm = con.Acm;
                TestHelper.Assert(acm.Timeout == TimeSpan.FromSeconds(15));
                TestHelper.Assert(acm.Close == AcmClose.OnIdleForceful);
                TestHelper.Assert(acm.Heartbeat == AcmHeartbeat.Off);

                con.Acm = new Acm(TimeSpan.FromSeconds(1),
                                  AcmClose.OnInvocationAndIdle,
                                  AcmHeartbeat.Always);
                acm = con.Acm;
                TestHelper.Assert(acm.Timeout == TimeSpan.FromSeconds(1));
                TestHelper.Assert(acm.Close == AcmClose.OnInvocationAndIdle);
                TestHelper.Assert(acm.Heartbeat == AcmHeartbeat.Always);

                proxy.StartHeartbeatCount();
                proxy.WaitForHeartbeatCount(2);

                var t1 = new TaskCompletionSource<object?>();
                var t2 = new TaskCompletionSource<object?>();
                con.Closed += (sender, args) => t1.SetResult(null);
                con.Closed += (sender, args) => t2.SetResult(null);

                con.Close(ConnectionClose.Gracefully);
                TestHelper.Assert(t1.Task.Result == null);
                TestHelper.Assert(t2.Task.Result == null);

                try
                {
                    con.ThrowException();
                    TestHelper.Assert(false);
                }
                catch (ConnectionClosedLocallyException)
                {
                }

                var t3 = new TaskCompletionSource<object?>();
                con.Closed += (sender, args) => t3.SetResult(null);
                TestHelper.Assert(t3.Task.Result == null);

                con.HeartbeatReceived += (sender, args) => TestHelper.Assert(false);

                foreach ((string close, string hearbeat) in new (string, string)[]
                                                                {
                                                                    ("Off",  "Off"),
                                                                    ("OnIdle", "OnDispatch"),
                                                                    ("OnInvocation", "OnIdle"),
                                                                    ("OnInvocationAndIdle", "Always"),
                                                                    ("OnIdleForceful", "Off")
                                                                })
                {
                    using var communicator = new Communicator(
                        new Dictionary<string, string>(proxy.Communicator.GetProperties())
                        {
                            ["Ice.ACM.Client.Close"] = close,
                            ["Ice.ACM.Client.Heartbeat"] = hearbeat
                        });

                    proxy = ITestIntfPrx.Parse(proxy.ToString()!, communicator);
                    Connection? connection = proxy.GetConnection()!;
                    TestHelper.Assert(connection.Acm.Close == (AcmClose)Enum.Parse(typeof(AcmClose), close));
                    TestHelper.Assert(connection.Acm.Heartbeat ==
                        (AcmHeartbeat)Enum.Parse(typeof(AcmHeartbeat), hearbeat));
                }
            }
        }

        public static void allTests(TestHelper helper)
        {
            Communicator? communicator = helper.Communicator();
            TestHelper.Assert(communicator != null);
            var com = IRemoteCommunicatorPrx.Parse($"communicator:{helper.GetTestEndpoint(0)}", communicator);

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
                new SetAcmTest(com, helper)
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
            com.Shutdown();
            output.WriteLine("ok");
        }
    }
}
