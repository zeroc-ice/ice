// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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
