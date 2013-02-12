// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Demo;
using System;

public sealed class CallbackI : CallbackDisp_
{
    public override void initiateCallback(CallbackReceiverPrx proxy, Ice.Current current)
    {
        Console.WriteLine("initiating callback to: " + current.adapter.getCommunicator().proxyToString(proxy));
        try
        {
            proxy.callback();
        }
        catch(System.Exception ex)
        {
            Console.Error.WriteLine(ex);
        }
    }

    public override void shutdown(Ice.Current current)
    {
        Console.WriteLine("shutting down...");
        try
        {
            current.adapter.getCommunicator().shutdown();
        }
        catch(System.Exception ex)
        {
            Console.Error.WriteLine(ex);
        }
    }
}
