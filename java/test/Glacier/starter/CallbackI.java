// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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
        proxy.callback(current.context);
    }

    public void
    initiateCallbackEx(CallbackReceiverPrx proxy, Ice.Current current)
	throws CallbackException
    {
        proxy.callbackEx(current.context);
    }

    public void
    shutdown(Ice.Current current)
    {
        _communicator.shutdown();
    }

    private Ice.Communicator _communicator;
}
