// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public sealed class TestI : Test.TestIntfDisp_
{
    public
    TestI(Ice.ObjectAdapter adapter)
    {
        _adapter = adapter;
    }

    public override void
    shutdown(Ice.Current current)
    {
        _adapter.getCommunicator().shutdown();
    }

    private Ice.ObjectAdapter _adapter;
}
