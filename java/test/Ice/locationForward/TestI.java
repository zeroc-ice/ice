// **********************************************************************
//
// Copyright (c) 2002
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
    TestI(Ice.ObjectAdapter adapter, Ice.ObjectPrx fwd)
    {
        _adapter = adapter;
        _fwd = fwd;
    }

    public void
    shutdown(Ice.Current current)
    {
        _adapter.getCommunicator().shutdown();
        if(_fwd != null)
        {
            throw new Ice.LocationForward(_fwd);
        }
    }

    private Ice.ObjectAdapter _adapter;
    private Ice.ObjectPrx _fwd;
}
