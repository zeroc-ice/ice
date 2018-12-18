// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.interrupt;

import java.util.ArrayList;
import java.util.List;

class TestControllerI implements test.Ice.interrupt.Test.TestIntfController
{
    synchronized void addUpcallThread()
    {
        _threads.add(Thread.currentThread());
    }

    synchronized void removeUpcallThread()
    {
        _threads.remove(Thread.currentThread());
        //
        // Clear the interrupted state after removing the thread.
        //
        Thread.interrupted();
    }

    @Override
    synchronized public void interrupt(com.zeroc.Ice.Current current)
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
    public void holdAdapter(com.zeroc.Ice.Current current)
    {
        _adapter.hold();
    }

    @Override
    public void resumeAdapter(com.zeroc.Ice.Current current)
    {
        _adapter.activate();
    }

    TestControllerI(com.zeroc.Ice.ObjectAdapter adapter)
    {
        _adapter = adapter;
    }

    final private com.zeroc.Ice.ObjectAdapter _adapter;
    final private List<Thread> _threads = new ArrayList<>();
}
