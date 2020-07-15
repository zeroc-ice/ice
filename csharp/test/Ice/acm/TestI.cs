//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;
using System;

namespace ZeroC.Ice.Test.ACM
{
    public class RemoteCommunicator : IRemoteCommunicator
    {
        public IRemoteObjectAdapterPrx createObjectAdapter(int timeout, string? close, string? heartbeat, Current current)
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

        public void shutdown(Current current) => _ = current.Adapter.Communicator.ShutdownAsync();

    }

    public class RemoteObjectAdapter : IRemoteObjectAdapter
    {
        public RemoteObjectAdapter(ObjectAdapter adapter)
        {
            _adapter = adapter;
            _testIntf = _adapter.Add("test", new TestIntf(), ITestIntfPrx.Factory);
            _adapter.Activate();
        }

        public ITestIntfPrx getTestIntf(Current current) => _testIntf;

        public void deactivate(Current current) => _adapter.Dispose();

        private readonly ObjectAdapter _adapter;
        private readonly ITestIntfPrx _testIntf;
    }

    public class TestIntf : ITestIntf
    {
        public void sleep(int delay, Current current)
        {
            lock (this)
            {
                System.Threading.Monitor.Wait(this, TimeSpan.FromSeconds(delay));
            }
        }

        public void interruptSleep(Current current)
        {
            lock (this)
            {
                System.Threading.Monitor.PulseAll(this);
            }
        }

        public class HeartbeatCallbackI
        {
            public void Heartbeat()
            {
                lock (this)
                {
                    ++_count;
                    System.Threading.Monitor.PulseAll(this);
                }
            }

            public void WaitForCount(int count)
            {
                lock (this)
                {
                    while (_count < count)
                    {
                        System.Threading.Monitor.Wait(this);
                    }
                }
            }

            private int _count = 0;
        }

        public void startHeartbeatCount(Current current)
        {
            _callback = new HeartbeatCallbackI();
            current.Connection!.HeartbeatReceived += (sender, args) => _callback.Heartbeat();
        }

        public void waitForHeartbeatCount(int count, Current current)
        {
            TestHelper.Assert(_callback != null);
            _callback.WaitForCount(count);
        }

        private HeartbeatCallbackI? _callback;
    }
}
