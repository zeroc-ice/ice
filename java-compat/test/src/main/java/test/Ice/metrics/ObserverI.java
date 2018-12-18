// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.Ice.metrics;

class ObserverI implements Ice.Instrumentation.Observer
{
    synchronized public void
    reset()
    {
        total = 0;
        current = 0;
        failedCount = 0;
    }

    @Override
    synchronized public void
    attach()
    {
        ++total;
        ++current;
    }
    @Override
    synchronized public void
    detach()
    {
        --current;
    }
    @Override
    synchronized public void
    failed(String s)
    {
        ++failedCount;
    }

    synchronized int
    getTotal()
    {
        return total;
    }

    synchronized int
    getCurrent()
    {
        return current;
    }

    synchronized int
    getFailedCount()
    {
        return failedCount;
    }

    int total;
    int current;
    int failedCount;
};
