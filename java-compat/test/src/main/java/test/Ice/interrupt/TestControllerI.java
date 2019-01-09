// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.Ice.interrupt;

import java.util.ArrayList;
import java.util.List;

class TestControllerI extends test.Ice.interrupt.Test._TestIntfControllerDisp
{
    synchronized void
    addUpcallThread()
    {
        _threads.add(Thread.currentThread());
    }

    synchronized void
    removeUpcallThread()
    {
        _threads.remove(Thread.currentThread());
        //
        // Clear the interrupted state after removing the thread.
        //
        Thread.interrupted();
    }

    @Override
    synchronized public void
    interrupt(Ice.Current current)
        throws test.Ice.interrupt.Test.CannotInterruptException
    {
        if(_threads.isEmpty())
        {
            throw new test.Ice.interrupt.Test.CannotInterruptException();
        }
        for(Thread t : _threads)
        {
            t.interrupt();
        }
    }

    @Override
    public void
    holdAdapter(Ice.Current current)
    {
        _adapter.hold();
    }

    @Override
    public void
    resumeAdapter(Ice.Current current)
    {
        _adapter.activate();
    }

    TestControllerI(Ice.ObjectAdapter adapter)
    {
        _adapter = adapter;
    }

    final private Ice.ObjectAdapter _adapter;
    final private List<Thread> _threads = new ArrayList<Thread>();
}
