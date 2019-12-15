//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

public sealed class CallbackI : Test.Callback
{
    public void
    initiateCallback(CallbackReceiverPrx proxy, Ice.Current current)
    {
        proxy.callback(current.Context);
    }

    public void
    initiateCallbackEx(CallbackReceiverPrx proxy, Ice.Current current)
    {
        proxy.callbackEx(current.Context);
    }

    public void
    shutdown(Ice.Current current)
    {
        current.Adapter.Communicator.shutdown();
    }
}
