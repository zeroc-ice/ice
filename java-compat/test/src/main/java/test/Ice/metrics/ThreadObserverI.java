// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.Ice.metrics;

class ThreadObserverI extends ObserverI implements Ice.Instrumentation.ThreadObserver
{
    @Override
    public synchronized void
    reset()
    {
        super.reset();
        states = 0;
    }

    @Override
    public synchronized void
    stateChanged(Ice.Instrumentation.ThreadState o, Ice.Instrumentation.ThreadState n)
    {
        ++states;
    }

    int states;
};
