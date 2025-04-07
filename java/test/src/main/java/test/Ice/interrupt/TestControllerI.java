// Copyright (c) ZeroC, Inc.

package test.Ice.interrupt;

import com.zeroc.Ice.Current;
import com.zeroc.Ice.ObjectAdapter;

import test.Ice.interrupt.Test.CannotInterruptException;
import test.Ice.interrupt.Test.TestIntfController;

import java.util.ArrayList;
import java.util.List;

class TestControllerI implements TestIntfController {
    synchronized void addUpcallThread() {
        _threads.add(Thread.currentThread());
    }

    synchronized void removeUpcallThread() {
        _threads.remove(Thread.currentThread());
        //
        // Clear the interrupted state after removing the thread.
        //
        Thread.interrupted();
    }

    @Override
    public synchronized void interrupt(Current current)
            throws CannotInterruptException {
        if (_threads.isEmpty()) {
            throw new CannotInterruptException();
        }
        for (Thread t : _threads) {
            t.interrupt();
        }
    }

    @Override
    public void holdAdapter(Current current) {
        _adapter.hold();
    }

    @Override
    public void resumeAdapter(Current current) {
        _adapter.activate();
    }

    TestControllerI(ObjectAdapter adapter) {
        _adapter = adapter;
    }

    private final ObjectAdapter _adapter;
    private final List<Thread> _threads = new ArrayList<>();
}
