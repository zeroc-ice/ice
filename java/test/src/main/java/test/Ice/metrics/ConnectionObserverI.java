// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.metrics;

class ConnectionObserverI extends ObserverI implements Ice.Instrumentation.ConnectionObserver
{
    @Override
    public synchronized void 
    reset()
    {
        super.reset();
        received = 0;
        sent = 0;
    }

    @Override
    public synchronized void 
    sentBytes(int s)
    {
        sent += s;
    }

    @Override
    public synchronized void 
    receivedBytes(int s)
    {
        received += s;
    }
    
    int sent;
    int received;
};

