//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;
using System;
using System.Threading;

namespace ZeroC.Ice.Test.ACM
{
    public class RemoteCommunicator : IRemoteCommunicator
    {
        public IRemoteObjectAdapterPrx CreateObjectAdapter(int timeout, string? close, string? heartbeat, Current current)
        {
            Communicator communicator = current.Adapter.Communicator;
            string transport = communicator.GetProperty("Ice.Default.Transport") ?? "tcp";
            string host = communicator.GetProperty("Ice.Default.Host") ?? "127.0.0.1";

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

            ObjectAdapter adapter = communicator.CreateObjectAdapterWithEndpoints(name, $"{transport} -h \"{host}\"");
            return current.Adapter.AddWithUUID(new RemoteObjectAdapter(adapter), IRemoteObjectAdapterPrx.Factory);
        }

        public void Shutdown(Current current) => _ = current.Adapter.Communicator.ShutdownAsync();

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

        public ITestIntfPrx GetTestIntf(Current current) => _testIntf;

        public void Deactivate(Current current) => _adapter.Dispose();
    }

    public class TestIntf : ITestIntf
    {
        private HeartbeatCallback? _callback;
        private readonly object _mutex = new object();
        public void Sleep(int delay, Current current)
        {
            lock (_mutex)
            {
                Monitor.Wait(_mutex, TimeSpan.FromSeconds(delay));
            }
        }

        public void InterruptSleep(Current current)
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

        public void StartHeartbeatCount(Current current)
        {
            _callback = new HeartbeatCallback();
            current.Connection!.HeartbeatReceived += (sender, args) => _callback.Heartbeat();
        }

        public void WaitForHeartbeatCount(int count, Current current)
        {
            TestHelper.Assert(_callback != null);
            _callback.WaitForCount(count);
        }
    }
}
