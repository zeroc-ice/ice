// Copyright (c) ZeroC, Inc.

package test.Ice.metrics;

import com.zeroc.Ice.Instrumentation.ChildInvocationObserver;

class ChildInvocationObserverI extends ObserverI
        implements ChildInvocationObserver {
    @Override
    public synchronized void reset() {
        super.reset();
        replySize = 0;
    }

    @Override
    public synchronized void reply(int s) {
        replySize += s;
    }

    int replySize;
}
