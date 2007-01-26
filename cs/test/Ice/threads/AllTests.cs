// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Collections;
using System.Diagnostics;
using System.Threading;

public class AllTests
{
    private static void test(bool b)
    {
        if(!b)
        {
            throw new Exception();
        }
    }

    private class Callback
    {
        internal Callback()
        {
            _called = false;
        }
        
        public virtual bool check()
        {
            lock(this)
            {
                while(!_called)
                {
                    Monitor.Wait(this, TimeSpan.FromMilliseconds(5000));
                    
                    if(!_called)
                    {
                        return false; // Must be timeout.
                    }
                }
                
                return true;
            }
        }
        
        public virtual void called()
        {
            lock(this)
            {
                Debug.Assert(!_called);
                _called = true;
                Monitor.Pulse(this);
            }
        }
        
        private bool _called;
    }

    private class AMI_Adapter_waitForWakeupI : Test.AMI_Adapter_waitForWakeup
    {
        public override void ice_response(int id, bool notified)
        {
            _id = id;
            _notified = notified;
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public bool check()
        {
            return callback.check();
        }

        public int id()
        {
            return _id;
        }

        public bool notified()
        {
            return _notified;
        }

        private Callback callback = new Callback();
        private int _id;
        private bool _notified;
    }

    private class Counter
    {
        internal Counter(int target)
        {
            _target = target;
            _count = 0;
        }

        public bool bump(int timeout)
        {
            lock(this)
            {
                Debug.Assert(_count < _target);

                _count++;
                Monitor.PulseAll(this);

                long now = System.DateTime.Now.Ticks / 10000;
                long end = 0;
                if(timeout > 0)
                {
                    end = now + timeout;
                }

                while(_count < _target)
                {
                    if(timeout > 0)
                    {
                        Monitor.Wait(this, (int)(end - now));
                        now = System.DateTime.Now.Ticks / 10000;
                        if(now >= end)
                        {
                            break;
                        }
                    }
                    else
                    {
                        Monitor.Wait(this);
                    }
                }
                return _count == _target;
            }
        }

        private int _target;
        private int _count;
    }

    private class AMI_Server_pingI : Test.AMI_Server_ping
    {
        internal AMI_Server_pingI(Counter counter, int timeout)
        {
            _counter = counter;
            _timeout = timeout;
            _ok = false;
        }

        public override void ice_response()
        {
            _thread = Thread.CurrentThread;
            _ok = _counter.bump(_timeout);
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public bool check()
        {
            return callback.check();
        }

        public Thread thread()
        {
            return _thread;
        }

        public bool ok()
        {
            return _ok;
        }

        private Callback callback = new Callback();
        private Counter _counter;
        private int _timeout;
        private Thread _thread;
        private bool _ok;
    }

    public static Test.ServerPrx allTests(Ice.Communicator communicator)
    {
        string @ref = "server:default -p 12010 -t 10000";
        Ice.ObjectPrx @base = communicator.stringToProxy(@ref);
        test(@base != null);
        Test.ServerPrx server = Test.ServerPrxHelper.checkedCast(@base);

        Console.Out.Write("testing proxy configurations... ");
        Console.Out.Flush();
        {
            Ice.InitializationData id = new Ice.InitializationData();
            id.properties = communicator.getProperties().ice_clone_();
            id.properties.setProperty("Ice.ThreadPerConnection", "0");
            Ice.Communicator comm = Ice.Util.initialize(id);

            Ice.ObjectPrx proxy = comm.stringToProxy(@ref);
            test(!proxy.ice_isThreadPerConnection());

            proxy = proxy.ice_oneway();
            test(!proxy.ice_isThreadPerConnection());

            proxy = proxy.ice_threadPerConnection(true);
            test(proxy.ice_isThreadPerConnection());

            proxy = proxy.ice_twoway();
            test(proxy.ice_isThreadPerConnection());

            proxy = proxy.ice_threadPerConnection(false);
            test(!proxy.ice_isThreadPerConnection());

            comm.destroy();
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("testing connections... ");
        Console.Out.Flush();
        {
            //
            // Proxies with different settings for thread-per-connection should use different connections.
            //
            Ice.InitializationData id = new Ice.InitializationData();
            id.properties = communicator.getProperties().ice_clone_();
            id.properties.setProperty("Ice.ThreadPerConnection", "0");
            Ice.Communicator comm = Ice.Util.initialize(id);

            Ice.ObjectPrx proxy1 = comm.stringToProxy(@ref);
            test(!proxy1.ice_isThreadPerConnection());
            Ice.Connection conn1 = proxy1.ice_getConnection();
            proxy1.ice_ping();

            Ice.ObjectPrx proxy2 = proxy1.ice_threadPerConnection(true);
            test(proxy2.ice_isThreadPerConnection());
            Ice.Connection conn2 = proxy2.ice_getConnection();
            test(conn2 != conn1);
            proxy2.ice_ping();

            //
            // Verify connection re-use.
            //
            Ice.ObjectPrx proxy3 = comm.stringToProxy(@ref);
            test(!proxy3.ice_isThreadPerConnection());
            Ice.Connection conn3 = proxy3.ice_getConnection();
            test(conn3 == conn1);
            proxy3.ice_ping();

            Ice.ObjectPrx proxy4 = proxy3.ice_threadPerConnection(true);
            test(proxy4.ice_isThreadPerConnection());
            Ice.Connection conn4 = proxy4.ice_getConnection();
            test(conn4 == conn2);
            proxy4.ice_ping();

            comm.destroy();
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("testing object adapters... ");
        Console.Out.Flush();
        {
            Test.AdapterPrx[] adapters = server.getAdapters();
            Hashtable threadPerConnectionThreads = new Hashtable();
            Hashtable threadPoolThreads = new Hashtable();

            for(int i = 0; i < adapters.Length; ++i)
            {
                Test.AdapterPrx adapter = adapters[i];
                if(adapter.isThreadPerConnection())
                {
                    //
                    // For thread-per-connection, only one request can be dispatched at a time.
                    // Since the adapters that use a thread pool are configured to allow up to
                    // two threads, we can verify that the adapter is using thread-per-connection
                    // by attempting to make two concurrent requests.
                    //
                    AMI_Adapter_waitForWakeupI cb = new AMI_Adapter_waitForWakeupI();
                    adapter.reset();
                    adapter.waitForWakeup_async(cb, 250);
                    int tid = adapter.wakeup();
                    test(cb.check());
                    test(!cb.notified());
                    test(tid == cb.id()); // The thread id must be the same for both requests.
                    test(!threadPerConnectionThreads.Contains(tid));
                    threadPerConnectionThreads.Add(tid, null);

                    //
                    // Closing the connection and creating a new one should start a new thread
                    // in the server.
                    //
                    adapter.ice_getConnection().close(false);
                    int tid2 = adapter.getThreadId();
                    test(tid != tid2);
                    test(!threadPerConnectionThreads.Contains(tid2));
                    threadPerConnectionThreads.Add(tid2, null);
                }
                else
                {
                    AMI_Adapter_waitForWakeupI cb = new AMI_Adapter_waitForWakeupI();
                    adapter.reset();
                    adapter.waitForWakeup_async(cb, 250);
                    int tid = adapter.wakeup();
                    test(cb.check());
                    test(cb.notified());
                    test(tid != cb.id()); // The thread ids must be different for the requests.
                    test(!threadPoolThreads.Contains(tid));
                    threadPoolThreads.Add(tid, null);
                    test(!threadPoolThreads.Contains(cb.id()));
                    threadPoolThreads.Add(cb.id(), null);
                }
            }
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("testing client thread pool... ");
        Console.Out.Flush();
        {
            //
            // With two threads in the client-side thread pool, the AMI responses
            // should be dispatched concurrently.
            //
            Ice.InitializationData id = new Ice.InitializationData();
            id.properties = communicator.getProperties().ice_clone_();
            id.properties.setProperty("Ice.ThreadPerConnection", "0");
            id.properties.setProperty("Ice.ThreadPool.Client.SizeMax", "2");
            id.properties.setProperty("Ice.ThreadPool.Client.SizeWarn", "0");
            Ice.Communicator comm = Ice.Util.initialize(id);

            Test.ServerPrx srv = Test.ServerPrxHelper.checkedCast(comm.stringToProxy(@ref));
            test(!srv.ice_isThreadPerConnection());
            Counter counter = new Counter(2);
            AMI_Server_pingI cb1 = new AMI_Server_pingI(counter, 0);
            srv.ping_async(cb1);
            AMI_Server_pingI cb2 = new AMI_Server_pingI(counter, 0);
            srv.ping_async(cb2);
            cb1.check();
            cb2.check();
            test(cb1.ok());
            test(cb2.ok());
            test(cb1.thread() != cb2.thread());

            comm.destroy();
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("testing client thread-per-connection... ");
        Console.Out.Flush();
        {
            //
            // With thread-per-connection, AMI responses are dispatched synchronously by
            // the same thread.
            //
            Ice.InitializationData id = new Ice.InitializationData();
            id.properties = communicator.getProperties().ice_clone_();
            id.properties.setProperty("Ice.ThreadPerConnection", "1");
            Ice.Communicator comm = Ice.Util.initialize(id);

            Test.ServerPrx srv = Test.ServerPrxHelper.checkedCast(comm.stringToProxy(@ref));
            test(srv.ice_isThreadPerConnection());
            Counter counter = new Counter(2);
            AMI_Server_pingI cb1 = new AMI_Server_pingI(counter, 250);
            srv.ping_async(cb1);
            AMI_Server_pingI cb2 = new AMI_Server_pingI(counter, 250);
            srv.ping_async(cb2);
            cb1.check();
            cb2.check();
            //
            // One of the callbacks should have timed out.
            //
            test((cb1.ok() && !cb2.ok()) || (!cb1.ok() && cb2.ok()));
            test(cb1.thread() == cb2.thread());

            comm.destroy();
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("testing collocated invocations... ");
        Console.Out.Flush();
        {
            Test.AdapterPrx[] adapters = server.getAdapters();
            for(int i = 0; i < adapters.Length; ++i)
            {
                adapters[i].callSelf(adapters[i]);
            }
        }
        Console.Out.WriteLine("ok");

        return server;
    }
}
