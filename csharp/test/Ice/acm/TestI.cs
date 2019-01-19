//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice
{
    namespace acm
    {
        public class RemoteCommunicatorI : Test.RemoteCommunicatorDisp_
        {
            public override Test.RemoteObjectAdapterPrx
            createObjectAdapter(int timeout, int close, int heartbeat, Ice.Current current)
            {
                Ice.Communicator com = current.adapter.getCommunicator();
                Ice.Properties properties = com.getProperties();
                string protocol = properties.getPropertyWithDefault("Ice.Default.Protocol", "tcp");
                string host = properties.getPropertyWithDefault("Ice.Default.Host", "127.0.0.1");

                string name = System.Guid.NewGuid().ToString();
                if(timeout >= 0)
                {
                    properties.setProperty(name + ".ACM.Timeout", timeout.ToString());
                }
                if(close >= 0)
                {
                    properties.setProperty(name + ".ACM.Close", close.ToString());
                }
                if(heartbeat >= 0)
                {
                    properties.setProperty(name + ".ACM.Heartbeat", heartbeat.ToString());
                }
                properties.setProperty(name + ".ThreadPool.Size", "2");
                Ice.ObjectAdapter adapter = com.createObjectAdapterWithEndpoints(name, protocol + " -h \"" + host + "\"");
                return Test.RemoteObjectAdapterPrxHelper.uncheckedCast(
                    current.adapter.addWithUUID(new RemoteObjectAdapterI(adapter)));
            }

            public override void
            shutdown(Ice.Current current)
            {
                current.adapter.getCommunicator().shutdown();
            }
        }

        public class RemoteObjectAdapterI : Test.RemoteObjectAdapterDisp_
        {
            public RemoteObjectAdapterI(Ice.ObjectAdapter adapter)
            {
                _adapter = adapter;
                _testIntf = Test.TestIntfPrxHelper.uncheckedCast(_adapter.add(new TestI(), Util.stringToIdentity("test")));
                _adapter.activate();
            }

            public override Test.TestIntfPrx getTestIntf(Ice.Current current)
            {
                return _testIntf;
            }

            public override void activate(Ice.Current current)
            {
                _adapter.activate();
            }

            public override void hold(Ice.Current current)
            {
                _adapter.hold();
            }

            public override void deactivate(Ice.Current current)
            {
                try
                {
                    _adapter.destroy();
                }
                catch(Ice.ObjectAdapterDeactivatedException)
                {
                }
            }

            private Ice.ObjectAdapter _adapter;
            private Test.TestIntfPrx _testIntf;
        }

        public class TestI : Test.TestIntfDisp_
        {
            public override void sleep(int delay, Ice.Current current)
            {
                lock(this)
                {
                    System.Threading.Monitor.Wait(this, delay * 1000);
                }
            }

            public override void sleepAndHold(int delay, Ice.Current current)
            {
                lock(this)
                {
                    current.adapter.hold();
                    System.Threading.Monitor.Wait(this, delay * 1000);
                }
            }

            public override void interruptSleep(Ice.Current current)
            {
                lock(this)
                {
                    System.Threading.Monitor.PulseAll(this);
                }
            }

            class HeartbeatCallbackI
            {
                public void heartbeat(Ice.Connection c)
                {
                    lock(this)
                    {
                        ++_count;
                        System.Threading.Monitor.PulseAll(this);
                    }
                }

                public void waitForCount(int count)
                {
                    lock(this)
                    {
                        while(_count < count)
                        {
                            System.Threading.Monitor.Wait(this);
                        }
                    }
                }

                private int _count = 0;
            }

            public override void startHeartbeatCount(Ice.Current current)
            {
                _callback = new HeartbeatCallbackI();
                current.con.setHeartbeatCallback(_callback.heartbeat);
            }

            public override void waitForHeartbeatCount(int count, Ice.Current current)
            {
                System.Diagnostics.Debug.Assert(_callback != null);
                _callback.waitForCount(count);
            }

            private HeartbeatCallbackI _callback;
        }

    }
}
