// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

public class TestI extends _TestDisp
{
    public
    TestI(Ice.ObjectAdapter adapter)
    {
        _adapter = adapter;
    }

    public void
    shutdown(Ice.Current current)
    {
        _adapter.getCommunicator().shutdown();
    }

    private Ice.ObjectAdapter _adapter;
}
