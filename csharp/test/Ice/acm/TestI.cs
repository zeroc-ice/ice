//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Ice.acm.Test;

namespace Ice
{
    namespace acm
    {
        public class RemoteCommunicatorI : RemoteCommunicator
        {
            public RemoteObjectAdapterPrx
            createObjectAdapter(int timeout, int close, int heartbeat, Ice.Current current)
            {
                Communicator com = current.adapter.getCommunicator();
                Properties properties = com.getProperties();
                string protocol = properties.getPropertyWithDefault("Ice.Default.Protocol", "tcp");
                string host = properties.getPropertyWithDefault("Ice.Default.Host", "127.0.0.1");

                string name = System.Guid.NewGuid().ToString();
                if (timeout >= 0)
                {
                    properties.setProperty(name + ".ACM.Timeout", timeout.ToString());
                }
                if (close >= 0)
                {
                    properties.setProperty(name + ".ACM.Close", close.ToString());
                }
                if (heartbeat >= 0)
                {
                    properties.setProperty(name + ".ACM.Heartbeat", heartbeat.ToString());
                }
                properties.setProperty(name + ".ThreadPool.Size", "2");
                ObjectAdapter adapter = com.createObjectAdapterWithEndpoints(name, protocol + " -h \"" + host + "\"");
                RemoteObjectAdapter remoteObjectAdapter = new RemoteObjectAdapterI(adapter);
                return RemoteObjectAdapterPrxHelper.uncheckedCast(current.adapter.Add(remoteObjectAdapter));
            }

            public void
            shutdown(Current current)
            {
                current.adapter.getCommunicator().shutdown();
            }
        }

        public class RemoteObjectAdapterI : RemoteObjectAdapter
        {
            public RemoteObjectAdapterI(ObjectAdapter adapter)
            {
                _adapter = adapter;
                _testIntf = TestIntfPrxHelper.uncheckedCast(_adapter.Add(new TestI(), Util.stringToIdentity("test")));
                _adapter.activate();
            }

            public TestIntfPrx getTestIntf(Current current)
            {
                return _testIntf;
            }

            public void activate(Current current)
            {
                _adapter.activate();
            }

            public void hold(Current current)
            {
                _adapter.hold();
            }

            public void deactivate(Current current)
            {
                try
                {
                    _adapter.destroy();
                }
                catch (ObjectAdapterDeactivatedException)
                {
                }
            }

            private ObjectAdapter _adapter;
            private TestIntfPrx _testIntf;
        }

        public class TestI : TestIntf
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
                    current.adapter.hold();
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
                current.con.setHeartbeatCallback(_callback.heartbeat);
            }

            public void waitForHeartbeatCount(int count, Current current)
            {
                System.Diagnostics.Debug.Assert(_callback != null);
                _callback.waitForCount(count);
            }

            private HeartbeatCallbackI _callback;
        }

    }
}
