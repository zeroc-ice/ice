// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.Ice.faultTolerance;

import test.Ice.faultTolerance.Test._TestIntfDisp;

public final class TestI extends _TestIntfDisp
{
    public
    TestI(int port)
    {
        _pseudoPid = port; // We use the port number instead of the process ID in Java.
    }

    @Override
    public void
    shutdown(Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }

    @Override
    public void
    abort(Ice.Current current)
    {
        Runtime.getRuntime().halt(0);
    }

    @Override
    public void
    idempotentAbort(Ice.Current current)
    {
        Runtime.getRuntime().halt(0);
    }

    @Override
    public int
    pid(Ice.Current current)
    {
        return _pseudoPid;
    }

    private int _pseudoPid;
}
