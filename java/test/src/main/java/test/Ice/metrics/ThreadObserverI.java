// Copyright (c) ZeroC, Inc.

package test.Ice.metrics;

import com.zeroc.Ice.Instrumentation.ThreadObserver;
import com.zeroc.Ice.Instrumentation.ThreadState;

class ThreadObserverI extends ObserverI implements ThreadObserver {
    @Override
    public synchronized void reset() {
        super.reset();
        states = 0;
    }

    @Override
    public synchronized void stateChanged(
            ThreadState o,
            ThreadState n) {
        ++states;
    }

    int states;
}
