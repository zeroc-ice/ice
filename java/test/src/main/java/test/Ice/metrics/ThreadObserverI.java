// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.metrics;

class ThreadObserverI extends ObserverI implements com.zeroc.Ice.Instrumentation.ThreadObserver
{
    @Override
    public synchronized void reset()
    {
        super.reset();
        states = 0;
    }

    @Override
    public synchronized void stateChanged(com.zeroc.Ice.Instrumentation.ThreadState o,
                                          com.zeroc.Ice.Instrumentation.ThreadState n)
    {
        ++states;
    }

    int states;
}
