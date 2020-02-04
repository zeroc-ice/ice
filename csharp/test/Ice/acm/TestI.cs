//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Ice.acm.Test;

namespace Ice.acm
{
    public class RemoteCommunicator : IRemoteCommunicator
    {
        public IRemoteObjectAdapterPrx
        createObjectAdapter(int timeout, int close, int heartbeat, Current current)
        {
            Communicator communicator = current.Adapter.Communicator;
            string protocol = communicator.GetProperty("Ice.Default.Protocol") ?? "tcp";
            string host = communicator.GetProperty("Ice.Default.Host") ?? "127.0.0.1";

            string name = System.Guid.NewGuid().ToString();
            if (timeout >= 0)
            {
                communicator.SetProperty($"{name}.ACM.Timeout", timeout.ToString());
            }

            if (close >= 0)
            {
                communicator.SetProperty($"{name}.ACM.Close", close.ToString());
            }

            if (heartbeat >= 0)
            {
                communicator.SetProperty($"{name}.ACM.Heartbeat", heartbeat.ToString());
            }
            communicator.SetProperty($"{name}.ThreadPool.Size", "2");
            ObjectAdapter adapter = communicator.CreateObjectAdapterWithEndpoints(name, $"{protocol} -h \"{host}\"");
            return current.Adapter.AddWithUUID(new RemoteObjectAdapter(adapter), IRemoteObjectAdapterPrx.Factory);
        }

        public void
        shutdown(Current current) => current.Adapter.Communicator.Shutdown();

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

        public void activate(Current current) => _adapter.Activate();

        public void hold(Current current) => _adapter.Hold();

        public void deactivate(Current current)
        {
            try
            {
                _adapter.Destroy();
            }
            catch (ObjectAdapterDeactivatedException)
            {
            }
        }

        private ObjectAdapter _adapter;
        private ITestIntfPrx _testIntf;
    }

    public class TestIntf : ITestIntf
    {
        public void sleep(int delay, Current current)
        {
            lock (this)
            {
                System.Threading.Monitor.Wait(this, delay * 1000);
            }
        }

        public void sleepAndHold(int delay, Current current)
        {
            lock (this)
            {
                current.Adapter.Hold();
                System.Threading.Monitor.Wait(this, delay * 1000);
            }
        }

        public void interruptSleep(Current current)
        {
            lock (this)
            {
                System.Threading.Monitor.PulseAll(this);
            }
        }

        class HeartbeatCallbackI
        {
            public void heartbeat(Connection c)
            {
                lock (this)
                {
                    ++_count;
                    System.Threading.Monitor.PulseAll(this);
                }
            }

            public void waitForCount(int count)
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
            current.Connection.SetHeartbeatCallback(_callback.heartbeat);
        }

        public void waitForHeartbeatCount(int count, Current current)
        {
            System.Diagnostics.Debug.Assert(_callback != null);
            _callback.waitForCount(count);
        }

        private HeartbeatCallbackI _callback;
    }
}
