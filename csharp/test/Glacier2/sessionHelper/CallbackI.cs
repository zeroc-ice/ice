//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

public sealed class CallbackI : Test.CallbackDisp_
{
    public override void
    initiateCallback(CallbackReceiverPrx proxy, Ice.Current current)
    {
        proxy.callback(current.ctx);
    }

    public override void
    initiateCallbackEx(CallbackReceiverPrx proxy, Ice.Current current)
    {
        proxy.callbackEx(current.ctx);
    }

    public override void
    shutdown(Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }
}
