// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************


public sealed class CallbackI : Callback
{
    public override void initiateCallback(CallbackReceiverPrx proxy, Ice.Current current)
    {
        System.Console.Out.WriteLine("initiating callback");
        proxy.callback(current.ctx);
    }
    
    public override void shutdown(Ice.Current current)
    {
        System.Console.Out.WriteLine("Shutting down...");
        current.adapter.getCommunicator().shutdown();
    }
}
