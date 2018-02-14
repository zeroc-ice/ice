// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.dispatcher;

import test.Ice.dispatcher.Test.TestIntf;

public class TestI implements TestIntf
{
    private static void test(boolean b)
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
    public void op(com.zeroc.Ice.Current current)
    {
        test(_dispatcher.isDispatcherThread());
    }

    @Override
    public void
    sleep(int to, com.zeroc.Ice.Current current)
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
    public void opWithPayload(byte[] seq, com.zeroc.Ice.Current current)
    {
        test(_dispatcher.isDispatcherThread());
    }

    @Override
    public void shutdown(com.zeroc.Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }

    private Dispatcher _dispatcher;
}
