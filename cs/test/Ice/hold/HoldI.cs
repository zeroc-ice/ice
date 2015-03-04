// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System.Diagnostics;
using Test;

public sealed class HoldI : HoldDisp_
{
    private static void test(bool b)
    {
        if(!b)
        {
            throw new System.Exception();
        }
    }

    public HoldI(Ice.ObjectAdapter adapter)
    {
        _adapter = adapter;
    }

    public override void
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
            new DelayedTask(milliSeconds, delegate()
                {
                    try
                    {
                        putOnHold(0, null);
                    }
                    catch(Ice.ObjectAdapterDeactivatedException)
                    {
                    }
                });
        }
    }

    public override void
    waitForHold(Ice.Current current)
    {
        lock(this)
        {
            if(++_waitForHold == 1)
            {
                new DelayedTask(1, delegate()
                    {
                        while(true)
                        {
                            lock(this)
                            {
                                if(--_waitForHold < 0)
                                {
                                    return;
                                }
                            }
                            try
                            {
                                current.adapter.waitForHold();
                                current.adapter.activate();
                            }
                            catch(Ice.ObjectAdapterDeactivatedException)
                            {
                                //
                                // This shouldn't occur. The test ensures all the waitForHold timers are
                                // finished before shutting down the communicator.
                                //
                                test(false);
                            }
                        }
                    });
            }
        }
    }

    public override int
    set(int value, int delay, Ice.Current current)
    {
        System.Threading.Thread.Sleep(delay);

        lock(this)
        {
            int tmp = _last;
            _last = value;
            return tmp;
        }
    }

    public override void
    setOneway(int value, int expected, Ice.Current current)
    {
        lock(this)
        {
            test(_last == expected);
            _last = value;
        }
    }

    public override void
    shutdown(Ice.Current current)
    {
        _adapter.hold();
        _adapter.getCommunicator().shutdown();
    }

    //
    // The .NET Compact Framework doesn't support the System.Timers.Timer class,
    // but a simple thread does what we need in this test.
    //
    private delegate void Task();

    private class DelayedTask
    {
        internal DelayedTask(int timeout, Task task)
        {
            _timeout = timeout;
            _task = task;
            System.Threading.Thread t = new System.Threading.Thread(new System.Threading.ThreadStart(run));
            t.Start();
        }

        private void run()
        {
            System.Threading.Thread.Sleep(_timeout);
            _task();
        }

        private int _timeout;
        private Task _task;
    }

    private Ice.ObjectAdapter _adapter;
    private int _last = 0;
    private int _waitForHold = 0;
}
