// Copyright (c) ZeroC, Inc. All rights reserved.

using Test;
using System;
using System.Threading;

namespace ZeroC.Ice.Test.ACM
{
    public class RemoteCommunicator : IRemoteCommunicator
    {
        public IRemoteObjectAdapterPrx CreateObjectAdapter(
            int timeout,
            string? close,
            string? heartbeat,
            Current current,
            CancellationToken cancel)
        {
            Communicator communicator = current.Adapter.Communicator;
            string transport = communicator.GetProperty("Test.Transport")!;
            string host = communicator.GetProperty("Test.Host")!;

            string name = Guid.NewGuid().ToString();
            if (timeout >= 0)
            {
                communicator.SetProperty($"{name}.ACM.Timeout", $"{timeout}s");
            }

            if (close is string closeValue)
            {
                communicator.SetProperty($"{name}.ACM.Close", Enum.Parse<AcmClose>(closeValue).ToString());
            }

            if (heartbeat is string heartbeatValue)
            {
                communicator.SetProperty($"{name}.ACM.Heartbeat", Enum.Parse<AcmHeartbeat>(heartbeatValue).ToString());
            }

            bool ice1 = TestHelper.GetTestProtocol(communicator.GetProperties()) == Protocol.Ice1;
            if (!ice1 && host.Contains(':'))
            {
                host = $"[{host}]";
            }
            ObjectAdapter adapter = communicator.CreateObjectAdapterWithEndpoints(name,
                ice1 ? $"{transport} -h \"{host}\"" : $"ice+{transport}://{host}:0");

            return current.Adapter.AddWithUUID(new RemoteObjectAdapter(adapter), IRemoteObjectAdapterPrx.Factory);
        }

        public void Shutdown(Current current, CancellationToken cancel) =>
            _ = current.Adapter.Communicator.ShutdownAsync();
    }

    public class RemoteObjectAdapter : IRemoteObjectAdapter
    {
        private readonly ObjectAdapter _adapter;
        private readonly ITestIntfPrx _testIntf;

        public RemoteObjectAdapter(ObjectAdapter adapter)
        {
            _adapter = adapter;
            _testIntf = _adapter.Add("test", new TestIntf(), ITestIntfPrx.Factory);
            _adapter.Activate();
        }

        public ITestIntfPrx GetTestIntf(Current current, CancellationToken cancel) => _testIntf;

        public void Deactivate(Current current, CancellationToken cancel) => _adapter.Dispose();
    }

    public class TestIntf : ITestIntf
    {
        private HeartbeatCallback? _callback;
        private readonly object _mutex = new object();
        public void Sleep(int delay, Current current, CancellationToken cancel)
        {
            lock (_mutex)
            {
                Monitor.Wait(_mutex, TimeSpan.FromSeconds(delay));
            }
        }

        public void InterruptSleep(Current current, CancellationToken cancel)
        {
            lock (_mutex)
            {
                Monitor.PulseAll(_mutex);
            }
        }

        public class HeartbeatCallback
        {
            private int _count;
            private readonly object _mutex = new object();

            public void Heartbeat()
            {
                lock (_mutex)
                {
                    ++_count;
                    Monitor.PulseAll(_mutex);
                }
            }

            public void WaitForCount(int count)
            {
                lock (_mutex)
                {
                    while (_count < count)
                    {
                        Monitor.Wait(_mutex);
                    }
                }
            }
        }

        public void StartHeartbeatCount(Current current, CancellationToken cancel)
        {
            _callback = new HeartbeatCallback();
            current.Connection.PingReceived += (sender, args) => _callback.Heartbeat();
        }

        public void WaitForHeartbeatCount(int count, Current current, CancellationToken cancel)
        {
            TestHelper.Assert(_callback != null);
            _callback.WaitForCount(count);
        }
    }
}
