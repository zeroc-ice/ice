// **********************************************************************
//
// Copyright (c) 2002
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

final class CallbackI extends Callback
{
    CallbackI(Ice.Communicator communicator)
    {
        _communicator = communicator;
    }

    public void
    initiateCallback(CallbackReceiverPrx proxy, Ice.Current current)
    {
	proxy.callback(current.ctx);
    }

    public void
    initiateCallbackEx(CallbackReceiverPrx proxy, Ice.Current current)
	throws CallbackException
    {
        proxy.callbackEx(current.ctx);
    }

    public void
    shutdown(Ice.Current current)
    {
        _communicator.shutdown();
    }

    private Ice.Communicator _communicator;
}
