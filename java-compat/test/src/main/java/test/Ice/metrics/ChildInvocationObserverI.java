// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

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
