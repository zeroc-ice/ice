// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

public final class CallbackI extends Callback
{
    CallbackI(Ice.Communicator communicator)
    {
        _communicator = communicator;
    }

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
        _communicator.shutdown();
    }

    private Ice.Communicator _communicator;
}
