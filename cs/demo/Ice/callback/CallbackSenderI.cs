// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Demo;

public sealed class CallbackSenderI : CallbackSenderDisp_
{
    public override void initiateCallback(CallbackReceiverPrx proxy, Ice.Current current)
    {
        System.Console.Out.WriteLine("initiating callback");
        try
        {
            proxy.callback();
        }
        catch(System.Exception ex)
        {
            System.Console.Error.WriteLine(ex);
        }
    }
    
    public override void shutdown(Ice.Current current)
    {
        System.Console.Out.WriteLine("Shutting down...");
        try
        {
            current.adapter.getCommunicator().shutdown();
        }
        catch(System.Exception ex)
        {
            System.Console.Error.WriteLine(ex);
        }
    }
}
