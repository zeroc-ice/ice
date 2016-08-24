// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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

    static class Counter
    {
        Counter(int v)
        {
            value = v;
        }

        int value;
    }

    public void waitForHeartbeat(int count, com.zeroc.Ice.Current current)
    {
        final Counter c = new Counter(count);
        current.con.setHeartbeatCallback(con ->
            {
                synchronized(c)
                {
                    --c.value;
                    c.notifyAll();
                }
            });

        synchronized(c)
        {
            while(c.value > 0)
            {
                try
                {
                    c.wait();
                }
                catch(InterruptedException ex)
                {
                }
            }
        }
    }
}
