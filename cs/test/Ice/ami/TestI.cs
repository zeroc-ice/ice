// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Test;

public class TestI : TestIntfDisp_
{
    public TestI()
    {
    }

    override public void
    op(Ice.Current current)
    {
    }

    override public int
    opWithResult(Ice.Current current)
    {
        return 15;
    }

    override public void
    opWithUE(Ice.Current current)
    {
        throw new TestIntfException();
    }

    override public void
    opWithPayload(byte[] seq, Ice.Current current)
    {
    }

    override public void
    shutdown(Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }
}

public class TestControllerI : TestIntfControllerDisp_
{
    override public void
    holdAdapter(Ice.Current current)
    {
        _adapter.hold();
    }
    
    override public void
    resumeAdapter(Ice.Current current)
    {
        _adapter.activate();
    }
    
    public
    TestControllerI(Ice.ObjectAdapter adapter)
    {
        _adapter = adapter;
    }

    private Ice.ObjectAdapter _adapter;
};