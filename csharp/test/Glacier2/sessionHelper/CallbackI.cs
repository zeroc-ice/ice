//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

public sealed class CallbackI : Test.Callback
{
    public void
    initiateCallback(CallbackReceiverPrx proxy, Ice.Current current)
    {
        proxy.callback(current.ctx);
    }

    public void
    initiateCallbackEx(CallbackReceiverPrx proxy, Ice.Current current)
    {
        proxy.callbackEx(current.ctx);
    }

    public void
    shutdown(Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }
}
