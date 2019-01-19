//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.metrics;

class ChildInvocationObserverI extends ObserverI implements Ice.Instrumentation.ChildInvocationObserver
{
    @Override
    public synchronized void
    reset()
    {
        super.reset();
        replySize = 0;
    }

    @Override
    public synchronized void
    reply(int s)
    {
        replySize += s;
    }

    int replySize;
};
