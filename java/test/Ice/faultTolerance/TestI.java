// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

// Ice version 0.0.1

public final class TestI extends _TestDisp
{
    public
    TestI(Ice.ObjectAdapter adapter)
    {
        _adapter = adapter;
    }

    public void
    abort(Ice.Current current)
    {
        System.exit(1);
    }

    public void
    nonmutatingAbort(Ice.Current current)
    {
        System.exit(1);
    }

    public int
    pid(Ice.Current current)
    {
        return (int)(System.currentTimeMillis() % 65535);
    }

    public void
    shutdown(Ice.Current current)
    {
        _adapter.getCommunicator().shutdown();
    }

    private Ice.ObjectAdapter _adapter;
}
