// Copyright (c) ZeroC, Inc.

package test.Ice.metrics;

class ChildInvocationObserverI extends ObserverI
        implements com.zeroc.Ice.Instrumentation.ChildInvocationObserver {
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
