// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
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
