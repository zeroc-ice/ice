// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.acm;

import test.Ice.acm.Test.TestIntf;

public class TestI implements TestIntf
{
    public void sleep(int delay, com.zeroc.Ice.Current current)
    {
        synchronized(this)
        {
            try
            {
                wait(delay * 1000);
            }
            catch(java.lang.InterruptedException ex)
            {
            }
        }
    }

    public void sleepAndHold(int delay, com.zeroc.Ice.Current current)
    {
        synchronized(this)
        {
            try
            {
                current.adapter.hold();
                wait(delay * 1000);
            }
            catch(java.lang.InterruptedException ex)
            {
            }
        }
    }

    public void interruptSleep(com.zeroc.Ice.Current current)
    {
        synchronized(this)
        {
            notifyAll();
        }
    }

    public void startHeartbeatCount(com.zeroc.Ice.Current current)
    {
        _counter = new Counter();
        current.con.setHeartbeatCallback(con ->
            {
                synchronized(_counter)
                {
                    ++_counter.value;
                    _counter.notifyAll();
                }
            });
    }

    public void waitForHeartbeatCount(int count, com.zeroc.Ice.Current current)
    {
        assert(_counter != null);
        synchronized(_counter)
        {
            while(_counter.value < count)
            {
                try
                {
                    _counter.wait();
                }
                catch(InterruptedException ex)
                {
                }
            }
        }
    }

    static class Counter
    {
        int value;
    }

    private Counter _counter;
}
