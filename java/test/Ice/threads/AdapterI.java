// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public final class AdapterI extends Test._AdapterDisp
{
    public
    AdapterI(ThreadHookI hook, boolean threadPerConnection)
    {
        _threadHook = hook;
        _threadPerConnection = threadPerConnection;
    }

    public boolean
    isThreadPerConnection(Ice.Current current)
    {
        return _threadPerConnection;
    }

    public int
    getThreadId(Ice.Current current)
    {
        int tid = _threadHook.getThreadNum();
        test(tid != -1);
        return tid;
    }

    public void
    reset(Ice.Current current)
    {
        _waiting = false;
        _notified = false;
    }

    public synchronized int
    waitForWakeup(int timeout, Ice.BooleanHolder notified, Ice.Current current)
    {
        int tid = _threadHook.getThreadNum();
        test(tid != -1);

        _waiting = true;
        notifyAll();
        test(!_notified);
        try
        {
            wait(timeout);
            notified.value = _notified;
        }
        catch(InterruptedException ex)
        {
        }

        return tid;
    }

    public synchronized int
    wakeup(Ice.Current current)
    {
        int tid = _threadHook.getThreadNum();
        test(tid != -1);

        while(!_waiting)
        {
            try
            {
                wait();
            }
            catch(InterruptedException ex)
            {
            }
        }
        _notified = true;
        notifyAll();

        return tid;
    }

    public void
    callSelf(Test.AdapterPrx proxy, Ice.Current current)
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
        catch(Ice.CollocationOptimizationException ex)
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
        catch(Ice.CollocationOptimizationException ex)
        {
            // Expected.
        }
    }

    private static void
    test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    private ThreadHookI _threadHook;
    private boolean _threadPerConnection;
    private boolean _waiting;
    private boolean _notified;
}
