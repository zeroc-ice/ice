// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

public final class TestI extends _TestDisp
{
    public
    TestI(Ice.ObjectAdapter adapter)
    {
        _adapter = adapter;
	_pseudoPid = (new java.util.Random()).nextInt();
    }

    public void
    shutdown(Ice.Current current)
    {
        _adapter.getCommunicator().shutdown();
    }

    public void
    abort(Ice.Current current)
    {
	Runtime.getRuntime().halt(1);
    }

    public void
    idempotentAbort(Ice.Current current)
    {
	Runtime.getRuntime().halt(1);
    }

    public void
    nonmutatingAbort(Ice.Current current)
    {
	Runtime.getRuntime().halt(1);
    }

    public int
    pid(Ice.Current current)
    {
        return _pseudoPid;
    }

    private int _pseudoPid;
    private Ice.ObjectAdapter _adapter;
}
