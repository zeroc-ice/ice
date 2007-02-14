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

public class ThreadHookI : Ice.ThreadNotification
{
    public ThreadHookI()
    {
        _nextThreadId = 0;
    }

    public void start()
    {
        lock(this)
        {
            test(getThreadNumInternal() == -1);
            Thread t = Thread.CurrentThread;
            _threads.Add(t, _nextThreadId);
            _nextThreadId++;
        }
    }

    public void stop()
    {
        lock(this)
        {
            Thread t = Thread.CurrentThread;
            test(_threads.Contains(t));
            _threads.Remove(t);
        }
    }

    public int getThreadNum()
    {
        lock(this)
        {
            return getThreadNumInternal();
        }
    }

    public int activeThreads()
    {
        lock(this)
        {
            return _threads.Count;
        }
    }

    private int getThreadNumInternal()
    {
        Thread t = Thread.CurrentThread;
        if(!_threads.Contains(t))
        {
            return -1;
        }
        else
        {
            return (int)_threads[t];
        }
    }

    private static void test(bool b)
    {
        if(!b)
        {
            throw new Exception();
        }
    }

    private Hashtable _threads = new Hashtable();
    private int _nextThreadId;
}

public class AdapterI : Test.AdapterDisp_
{
    public AdapterI(ThreadHookI hook, bool threadPerConnection)
    {
        _threadHook = hook;
        _threadPerConnection = threadPerConnection;
    }

    public override bool isThreadPerConnection(Ice.Current current)
    {
        return _threadPerConnection;
    }

    public override int getThreadId(Ice.Current current)
    {
        int tid = _threadHook.getThreadNum();
        test(tid != -1);
        return tid;
    }

    public override void reset(Ice.Current current)
    {
        _waiting = false;
        _notified = false;
    }

    public override int waitForWakeup(int timeout, out bool notified, Ice.Current current)
    {
        lock(this)
        {
            int tid = _threadHook.getThreadNum();
            test(tid != -1);

            _waiting = true;
            Monitor.PulseAll(this);
            test(!_notified);

            Monitor.Wait(this, timeout);
            notified = _notified;

            return tid;
        }
    }

    public override int wakeup(Ice.Current current)
    {
        lock(this)
        {
            int tid = _threadHook.getThreadNum();
            test(tid != -1);

            while(!_waiting)
            {
                Monitor.Wait(this);
            }
            _notified = true;
            Monitor.PulseAll(this);

            return tid;
        }
    }

    public override void callSelf(Test.AdapterPrx proxy, Ice.Current current)
    {
        //
        // Make two invocations using different thread-per-connection settings.
        // In both cases, the invocations must be collocated.
        //

        proxy.ice_ping();
        try
        {
            //
            // A collocated invocation must not have a connection.
            //
            proxy.ice_getConnection();
            test(false);
        }
        catch(Ice.CollocationOptimizationException)
        {
            // Expected.
        }

        Test.AdapterPrx proxy2 =
            Test.AdapterPrxHelper.uncheckedCast(proxy.ice_threadPerConnection(!proxy.ice_isThreadPerConnection()));
        proxy2.ice_ping();
        try
        {
            //
            // A collocated invocation must not have a connection.
            //
            proxy2.ice_getConnection();
            test(false);
        }
        catch(Ice.CollocationOptimizationException)
        {
            // Expected.
        }
    }

    private static void test(bool b)
    {
        if(!b)
        {
            throw new Exception();
        }
    }

    private ThreadHookI _threadHook;
    private bool _threadPerConnection;
    private bool _waiting;
    private bool _notified;
}

public class ServerI : Test.ServerDisp_
{
    public ServerI(Ice.Communicator communicator)
    {
        _hook = new ThreadHookI();
        _adapters = new Test.AdapterPrx[4];

        Ice.Communicator comm;
        Ice.ObjectAdapter adapter;
        Ice.InitializationData id = new Ice.InitializationData();
        Test.AdapterPrx proxy;

        id.threadHook = _hook;

        Ice.Identity ident;

        //
        // Create an adapter that uses the communicator's thread pool.
        //
        id.properties = communicator.getProperties().ice_clone_();
        id.properties.setProperty("Ice.ThreadPerConnection", "0");
        id.properties.setProperty("Ice.ThreadPool.Server.SizeMax", "2");
        id.properties.setProperty("Ice.ThreadPool.Server.SizeWarn", "0");
        comm = Ice.Util.initialize(id);
        adapter = comm.createObjectAdapterWithEndpoints("Adapter1", "default");
        ident = comm.stringToIdentity("adapter1");
        proxy = Test.AdapterPrxHelper.uncheckedCast(adapter.add(new AdapterI(_hook, false), ident));
        adapter.activate();
        _adapters[0] = proxy;

        //
        // Create an adapter that uses the communicator's default setting for thread-per-connection.
        //
        id.properties = communicator.getProperties().ice_clone_();
        id.properties.setProperty("Ice.ThreadPerConnection", "1");
        id.properties.setProperty("Ice.ThreadPool.Server.SizeMax", "0");
        id.properties.setProperty("Ice.ThreadPool.Server.SizeWarn", "0");
        comm = Ice.Util.initialize(id);
        adapter = comm.createObjectAdapterWithEndpoints("Adapter2", "default");
        ident = comm.stringToIdentity("adapter2");
        proxy = Test.AdapterPrxHelper.uncheckedCast(adapter.add(new AdapterI(_hook, true), ident));
        adapter.activate();
        _adapters[1] = proxy;

        //
        // Create an adapter that uses a private thread pool.
        //
        id.properties = communicator.getProperties().ice_clone_();
        id.properties.setProperty("Ice.ThreadPerConnection", "1");
        id.properties.setProperty("Adapter3.ThreadPool.SizeMax", "2");
        id.properties.setProperty("Adapter3.ThreadPool.SizeWarn", "0");
        comm = Ice.Util.initialize(id);
        adapter = comm.createObjectAdapterWithEndpoints("Adapter3", "default");
        ident = comm.stringToIdentity("adapter3");
        proxy = Test.AdapterPrxHelper.uncheckedCast(adapter.add(new AdapterI(_hook, false), ident));
        adapter.activate();
        _adapters[2] = proxy;

        //
        // Create an adapter that uses an explicit setting for thread-per-connection.
        //
        id.properties = communicator.getProperties().ice_clone_();
        id.properties.setProperty("Ice.ThreadPerConnection", "0");
        id.properties.setProperty("Ice.ThreadPool.Server.SizeMax", "2");
        id.properties.setProperty("Ice.ThreadPool.Server.SizeWarn", "0");
        id.properties.setProperty("Adapter4.ThreadPerConnection", "1");
        comm = Ice.Util.initialize(id);
        adapter = comm.createObjectAdapterWithEndpoints("Adapter4", "default");
        ident = comm.stringToIdentity("adapter4");
        proxy = Test.AdapterPrxHelper.uncheckedCast(adapter.add(new AdapterI(_hook, true), ident));
        adapter.activate();
        _adapters[3] = proxy;
    }

    public override Test.AdapterPrx[] getAdapters(Ice.Current current)
    {
        return _adapters;
    }

    public override void ping(Ice.Current current)
    {
    }

    public override void shutdown(Ice.Current current)
    {
        for(int i = 0; i < _adapters.Length; ++i)
        {
            _adapters[i].ice_getCommunicator().destroy();
        }
        test(_hook.activeThreads() == 0);
        current.adapter.getCommunicator().shutdown();
    }

    private static void test(bool b)
    {
        if(!b)
        {
            throw new Exception();
        }
    }

    private Test.AdapterPrx[] _adapters;
    private ThreadHookI _hook;
}
