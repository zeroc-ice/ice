// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.metrics;

class DispatchObserverI extends ObserverI implements Ice.Instrumentation.DispatchObserver
{
    public synchronized void 
    reset()
    {
        super.reset();
        userExceptionCount = 0;
        replySize = 0;
    }

    public synchronized void 
    userException()
    {
        ++userExceptionCount;
    }

    public synchronized void 
    reply(int s)
    {
        replySize += s;
    }

    int userExceptionCount;
    int replySize;
};

