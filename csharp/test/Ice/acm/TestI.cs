// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Threading;
using System.Threading.Tasks;
using Test;

namespace ZeroC.Ice.Test.ACM
{
    public class RemoteCommunicator : IRemoteCommunicator
    {
        public IRemoteObjectAdapterPrx CreateObjectAdapter(
            int idleTimeout,
            bool keepAlive,
            Current current,
            CancellationToken cancel)
        {
            var communicator = new Communicator(
                new Dictionary<string, string>(current.Communicator.GetProperties())
                {
                    ["Ice.Warn.Connections"] = "0",
                    ["Ice.IdleTimeout"] = $"{idleTimeout}s",
                    ["Ice.KeepAlive"] = keepAlive ? "1" : "0"
                });

            var schedulerPair = new ConcurrentExclusiveSchedulerPair(TaskScheduler.Default);
            string endpoint = TestHelper.GetTestEndpoint(properties: communicator.GetProperties(), ephemeral: true);
            ObjectAdapter adapter = communicator.CreateObjectAdapterWithEndpoints("TestAdapter",
                endpoint,
                taskScheduler: schedulerPair.ExclusiveScheduler);

            return current.Adapter.AddWithUUID(new RemoteObjectAdapter(adapter), IRemoteObjectAdapterPrx.Factory);
        }

        public void Shutdown(Current current, CancellationToken cancel) =>
            _ = current.Communicator.ShutdownAsync();
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

        public void Deactivate(Current current, CancellationToken cancel) =>
            _adapter.Communicator.DisposeAsync().AsTask();
    }

    public class TestIntf : ITestIntf
    {
        private int _count;
        private readonly object _mutex = new object();

        public void Sleep(int delay, Current current, CancellationToken cancel)
        {
            Thread.Sleep(TimeSpan.FromSeconds(delay));
        }

        public void StartHeartbeatCount(Current current, CancellationToken cancel)
        {
            _count = 0;
            current.Connection.PingReceived += (sender, args) =>
            {
                lock (_mutex)
                {
                    ++_count;
                    Monitor.PulseAll(_mutex);
                }
            };
        }

        public void WaitForHeartbeatCount(int count, Current current, CancellationToken cancel)
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
}
