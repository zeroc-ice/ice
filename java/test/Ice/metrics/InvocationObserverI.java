// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.metrics;

class InvocationObserverI extends ObserverI implements Ice.Instrumentation.InvocationObserver
{
    public synchronized void 
    reset()
    {
        super.reset();
        retriedCount = 0;
        userExceptionCount = 0;
        if(remoteObserver != null)
        {
            remoteObserver.reset();
        }
    }

    public synchronized void 
    retried()
    {
        ++retriedCount;
    }

    public synchronized void 
    userException()
    {
        ++userExceptionCount;
    }

    public synchronized Ice.Instrumentation.RemoteObserver 
    getRemoteObserver(Ice.ConnectionInfo c, Ice.Endpoint e, int a, int b)
    {
        if(remoteObserver == null)
        {
            remoteObserver = new RemoteObserverI();
            remoteObserver.reset();
        }
        return remoteObserver;
    }

    int userExceptionCount;
    int retriedCount;

    RemoteObserverI remoteObserver = null;
};
