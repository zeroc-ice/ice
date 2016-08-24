
// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.hold;

import test.Ice.hold.Test._HoldDisp;

public final class HoldI extends _HoldDisp
{
    private static void
    test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    HoldI(java.util.Timer timer, Ice.ObjectAdapter adapter)
    {
        _timer = timer;
        _adapter = adapter;
        _last = 0;
    }

    @Override
    public void
    putOnHold(int milliSeconds, Ice.Current current)
    {
        if(milliSeconds < 0)
        {
            _adapter.hold();
        }
        else if(milliSeconds == 0)
        {
            _adapter.hold();
            _adapter.activate();
        }
        else
        {
            _timer.schedule(new java.util.TimerTask()
            {
                @Override
                public void run()
                {
                    try
                    {
                        putOnHold(0, null);
                    }
                    catch(Ice.ObjectAdapterDeactivatedException ex)
                    {
                    }
                }
            }, milliSeconds);
        }
    }

    @Override
    public void
    waitForHold(final Ice.Current current)
    {
        _timer.schedule(new java.util.TimerTask()
        {
            @Override
            public void run()
            {
                try
                {
                    current.adapter.waitForHold();

                    current.adapter.activate();
                }
                catch(Ice.ObjectAdapterDeactivatedException ex)
                {
                    //
                    // This shouldn't occur. The test ensures all the
                    // waitForHold timers are
                    // finished before shutting down the communicator.
                    //
                    test(false);
                }
            }
        }, 0);
    }


    @Override
    public int
    set(int value, int delay, Ice.Current current)
    {
        try
        {
            Thread.sleep(delay);
        }
        catch(java.lang.InterruptedException ex)
        {
        }

        synchronized(this)
        {
            int tmp = _last;
            _last = value;
            return tmp;
        }
    }

    @Override
    synchronized public void
    setOneway(int value, int expected, Ice.Current current)
    {
        test(_last == expected);
        _last = value;
    }

    @Override
    public void
    shutdown(Ice.Current current)
    {
        _adapter.hold();
        _adapter.getCommunicator().shutdown();
    }

    final private java.util.Timer _timer;
    final private Ice.ObjectAdapter _adapter;
    int _last = 0;
}
