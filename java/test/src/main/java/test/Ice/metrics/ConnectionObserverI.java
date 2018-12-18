// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.Ice.metrics;

class ConnectionObserverI extends ObserverI implements com.zeroc.Ice.Instrumentation.ConnectionObserver
{
    @Override
    public synchronized void reset()
    {
        super.reset();
        received = 0;
        sent = 0;
    }

    @Override
    public synchronized void sentBytes(int s)
    {
        sent += s;
    }

    @Override
    public synchronized void receivedBytes(int s)
    {
        received += s;
    }

    int sent;
    int received;
}
