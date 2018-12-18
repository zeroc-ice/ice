// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.IceGrid.simple;

import test.IceGrid.simple.Test._TestIntfDisp;

public class TestI extends _TestIntfDisp
{
    public
    TestI()
    {
    }

    @Override
    public void
    shutdown(Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }
}
