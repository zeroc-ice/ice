// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public final class CallbackI extends Callback
{
    public void
    initiateCallback(CallbackReceiverPrx proxy, Ice.Current current)
    {
        System.out.println("initiating callback");
        proxy.callback(current.ctx);
    }

    public void
    shutdown(Ice.Current current)
    {
        System.out.println("Shutting down...");
        current.adapter.getCommunicator().shutdown();
    }
}
