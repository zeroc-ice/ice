// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public class AllTests
{
    private static void
    test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    private static class Callback
    {
        Callback()
        {
            _called = false;
        }

        public synchronized boolean
        check()
        {
            while(!_called)
            {
                try
                {
                    wait(5000);
                }
                catch(InterruptedException ex)
                {
                    continue;
                }

                if(!_called)
                {
                    return false; // Must be timeout.
                }
            }

            _called = false;
            return true;
        }
        
        public synchronized void
        called()
        {
            assert(!_called);
            _called = true;
            notify();
        }

        private boolean _called;
    }

    private static class AMI_Adapter_waitForWakeupI extends Test.AMI_Adapter_waitForWakeup
    {
        public void
        ice_response(int id, boolean notified)
        {
            _id = id;
            _notified = notified;
            callback.called();
        }

        public void
        ice_exception(Ice.LocalException ex)
        {
            test(false);
        }

        boolean
        check()
        {
            return callback.check();
        }

        int
        id()
        {
            return _id;
        }

        boolean
        notified()
        {
            return _notified;
        }

        private Callback callback = new Callback();
        private int _id;
        private boolean _notified;
    }

    private static class Counter
    {
        Counter(int target)
        {
            _target = target;
            _count = 0;
        }

        synchronized boolean
        bump(int timeout)
        {
            assert(_count < _target);

            _count++;
            notifyAll();

            long now = System.currentTimeMillis();
            long end = 0;
            if(timeout > 0)
            {
                end = now + timeout;
            }

            while(_count < _target)
            {
                try
                {
                    if(timeout > 0)
                    {
                        wait(end - now);
                        now = System.currentTimeMillis();
                        if(now >= end)
                        {
                            break;
                        }
                    }
                    else
                    {
                        wait();
                    }
                }
                catch(InterruptedException ex)
                {
                }
            }
            return _count == _target;
        }

        private int _target;
        private int _count;
    }

    private static class AMI_Server_pingI extends Test.AMI_Server_ping
    {
        AMI_Server_pingI(Counter counter, int timeout)
        {
            _counter = counter;
            _timeout = timeout;
            _ok = false;
        }

        public void
        ice_response()
        {
            _thread = Thread.currentThread();
            _ok = _counter.bump(_timeout);
            callback.called();
        }

        public void
        ice_exception(Ice.LocalException ex)
        {
            test(false);
        }

        boolean
        check()
        {
            return callback.check();
        }

        Thread
        thread()
        {
            return _thread;
        }

        boolean
        ok()
        {
            return _ok;
        }

        private Callback callback = new Callback();
        private Counter _counter;
        private int _timeout;
        private Thread _thread;
        private boolean _ok;
    }

    public static Test.ServerPrx
    allTests(Ice.Communicator communicator)
    {
        final String ref = "server:default -p 12010 -t 10000";
        Ice.ObjectPrx base = communicator.stringToProxy(ref);
        test(base != null);
        Test.ServerPrx server = Test.ServerPrxHelper.checkedCast(base);

        System.out.print("testing proxy configurations... ");
        System.out.flush();
        {
            Ice.InitializationData id = new Ice.InitializationData();
            id.properties = communicator.getProperties()._clone();
            id.properties.setProperty("Ice.ThreadPerConnection", "0");
            Ice.Communicator comm = Ice.Util.initialize(id);

            Ice.ObjectPrx proxy = comm.stringToProxy(ref);
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
        System.out.println("ok");

        System.out.print("testing connections... ");
        System.out.flush();
        {
            //
            // Proxies with different settings for thread-per-connection should use different connections.
            //
            Ice.InitializationData id = new Ice.InitializationData();
            id.properties = communicator.getProperties()._clone();
            id.properties.setProperty("Ice.ThreadPerConnection", "0");
            Ice.Communicator comm = Ice.Util.initialize(id);

            Ice.ObjectPrx proxy1 = comm.stringToProxy(ref);
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
            Ice.ObjectPrx proxy3 = comm.stringToProxy(ref);
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
        System.out.println("ok");

        System.out.print("testing object adapters... ");
        System.out.flush();
        {
            Test.AdapterPrx[] adapters = server.getAdapters();
            java.util.Set threadPerConnectionThreads = new java.util.HashSet();
            java.util.Set threadPoolThreads = new java.util.HashSet();

            for(int i = 0; i < adapters.length; ++i)
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
                    test(!threadPerConnectionThreads.contains(new Integer(tid)));
                    threadPerConnectionThreads.add(new Integer(tid));

                    //
                    // Closing the connection and creating a new one should start a new thread
                    // in the server.
                    //
                    adapter.ice_getConnection().close(false);
                    int tid2 = adapter.getThreadId();
                    test(tid != tid2);
                    test(!threadPerConnectionThreads.contains(new Integer(tid2)));
                    threadPerConnectionThreads.add(new Integer(tid2));
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
                    test(!threadPoolThreads.contains(new Integer(tid)));
                    threadPoolThreads.add(new Integer(tid));
                    test(!threadPoolThreads.contains(new Integer(cb.id())));
                    threadPoolThreads.add(new Integer(cb.id()));
                }
            }
        }
        System.out.println("ok");

        System.out.print("testing client thread pool... ");
        System.out.flush();
        {
            //
            // With two threads in the client-side thread pool, the AMI responses
            // should be dispatched concurrently.
            //
            Ice.InitializationData id = new Ice.InitializationData();
            id.properties = communicator.getProperties()._clone();
            id.properties.setProperty("Ice.ThreadPerConnection", "0");
            id.properties.setProperty("Ice.ThreadPool.Client.SizeMax", "2");
            id.properties.setProperty("Ice.ThreadPool.Client.SizeWarn", "0");
            Ice.Communicator comm = Ice.Util.initialize(id);

            Test.ServerPrx srv = Test.ServerPrxHelper.checkedCast(comm.stringToProxy(ref));
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
        System.out.println("ok");

        System.out.print("testing client thread-per-connection... ");
        System.out.flush();
        {
            //
            // With thread-per-connection, AMI responses are dispatched synchronously by
            // the same thread.
            //
            Ice.InitializationData id = new Ice.InitializationData();
            id.properties = communicator.getProperties()._clone();
            id.properties.setProperty("Ice.ThreadPerConnection", "1");
            Ice.Communicator comm = Ice.Util.initialize(id);

            Test.ServerPrx srv = Test.ServerPrxHelper.checkedCast(comm.stringToProxy(ref));
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
        System.out.println("ok");

        System.out.print("testing collocated invocations... ");
        System.out.flush();
        {
            Test.AdapterPrx[] adapters = server.getAdapters();
            for(int i = 0; i < adapters.length; ++i)
            {
                adapters[i].callSelf(adapters[i]);
            }
        }
        System.out.println("ok");

        return server;
    }
}
