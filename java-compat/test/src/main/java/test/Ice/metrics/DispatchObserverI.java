// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.Ice.metrics;

class DispatchObserverI extends ObserverI implements Ice.Instrumentation.DispatchObserver
{
    @Override
    public synchronized void
    reset()
    {
        super.reset();
        userExceptionCount = 0;
        replySize = 0;
    }

    @Override
    public synchronized void
    userException()
    {
        ++userExceptionCount;
    }

    @Override
    public synchronized void
    reply(int s)
    {
        replySize += s;
    }

    int userExceptionCount;
    int replySize;
};
