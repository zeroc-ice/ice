// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.metrics;

class InvocationObserverI extends ObserverI implements com.zeroc.Ice.Instrumentation.InvocationObserver
{
    @Override
    public synchronized void reset()
    {
        super.reset();
        retriedCount = 0;
        userExceptionCount = 0;
        if(remoteObserver != null)
        {
            remoteObserver.reset();
        }
        if(collocatedObserver != null)
        {
            collocatedObserver.reset();
        }
    }

    @Override
    public synchronized void retried()
    {
        ++retriedCount;
    }

    @Override
    public synchronized void userException()
    {
        ++userExceptionCount;
    }

    @Override
    public synchronized com.zeroc.Ice.Instrumentation.RemoteObserver getRemoteObserver(
        com.zeroc.Ice.ConnectionInfo c,
        com.zeroc.Ice.Endpoint e,
        int a,
        int b)
    {
        if(remoteObserver == null)
        {
            remoteObserver = new RemoteObserverI();
            remoteObserver.reset();
        }
        return remoteObserver;
    }

    @Override
    public synchronized com.zeroc.Ice.Instrumentation.CollocatedObserver getCollocatedObserver(
        com.zeroc.Ice.ObjectAdapter adapter,
        int a,
        int b)
    {
        if(collocatedObserver == null)
        {
            collocatedObserver = new CollocatedObserverI();
            collocatedObserver.reset();
        }
        return collocatedObserver;
    }

    int userExceptionCount;
    int retriedCount;

    RemoteObserverI remoteObserver = null;
    CollocatedObserverI collocatedObserver = null;
}
