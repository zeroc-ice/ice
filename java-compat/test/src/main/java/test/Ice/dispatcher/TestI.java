// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.Ice.dispatcher;

import test.Ice.dispatcher.Test._TestIntfDisp;

public class TestI extends _TestIntfDisp
{
    private static void
    test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    TestI(Dispatcher dispatcher)
    {
        _dispatcher = dispatcher;
    }

    @Override
    public void
    op(Ice.Current current)
    {
        test(_dispatcher.isDispatcherThread());
    }

    @Override
    public void
    sleep(int to, Ice.Current current)
    {
        try
        {
            Thread.sleep(to);
        }
        catch(InterruptedException ex)
        {
            System.err.println("sleep interrupted");
        }
    }

    @Override
    public void
    opWithPayload(byte[] seq, Ice.Current current)
    {
        test(_dispatcher.isDispatcherThread());
    }

    @Override
    public void
    shutdown(Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }

    private Dispatcher _dispatcher;
}
