// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Test;
using System.Threading;

public class TestI : TestIntfDisp_
{
    private static void test(bool b)
    {
        if(!b)
        {
            throw new System.Exception();
        }
    }

    public TestI()
    {
    }

    override public void
    op(Ice.Current current)
    {
        test(Dispatcher.isDispatcherThread());
    }

    override public void
    opWithPayload(byte[] seq, Ice.Current current)
    {
        test(Dispatcher.isDispatcherThread());
    }

    public override void sleep(int to, Ice.Current current)
    {
        Thread.Sleep(to);
    }

    override public void
    shutdown(Ice.Current current)
    {
        test(Dispatcher.isDispatcherThread());
        current.adapter.getCommunicator().shutdown();
    }
}

public class TestControllerI : TestIntfControllerDisp_
{
    private static void test(bool b)
    {
        if(!b)
        {
            throw new System.Exception();
        }
    }

    override public void
    holdAdapter(Ice.Current current)
    {
        test(Dispatcher.isDispatcherThread());
        _adapter.hold();
    }
    
    override public void
    resumeAdapter(Ice.Current current)
    {
        test(Dispatcher.isDispatcherThread());
        _adapter.activate();
    }
    
    public
    TestControllerI(Ice.ObjectAdapter adapter)
    {
        _adapter = adapter;
    }

    private Ice.ObjectAdapter _adapter;
};
