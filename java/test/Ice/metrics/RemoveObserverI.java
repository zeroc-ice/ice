// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.metrics;

class RemoteObserverI extends ObserverI implements Ice.Instrumentation.RemoteObserver
{
    public synchronized void 
    reset()
    {
        super.reset();
        replySize = 0;
    }

    public synchronized void
    reply(int s)
    {
        replySize += s;
    }

    int replySize;
};

