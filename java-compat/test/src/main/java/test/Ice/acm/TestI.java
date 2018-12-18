// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.acm;
import test.Ice.acm.Test._TestIntfDisp;

public class TestI extends _TestIntfDisp
{
    public void sleep(int delay, Ice.Current current)
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

    public void sleepAndHold(int delay, Ice.Current current)
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

    public void interruptSleep(Ice.Current current)
    {
        synchronized(this)
        {
            notifyAll();
        }
    }

    public void startHeartbeatCount(Ice.Current current)
    {
        _counter = new Ice.Holder<Integer>(0);
        Ice.HeartbeatCallback callback = new Ice.HeartbeatCallback()
        {
            public void heartbeat(Ice.Connection connection)
            {
                synchronized(_counter)
                {
                    ++_counter.value;
                    _counter.notifyAll();
                }
            }

        };
        current.con.setHeartbeatCallback(callback);
    }

    public void waitForHeartbeatCount(int count, Ice.Current current)
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

    private Ice.Holder<Integer> _counter;
}
