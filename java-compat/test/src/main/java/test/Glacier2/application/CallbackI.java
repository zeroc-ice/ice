//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Glacier2.application;

import test.Glacier2.application.Test.CallbackReceiverPrx;
import test.Glacier2.application.Test._CallbackDisp;

final class CallbackI extends _CallbackDisp
{
    CallbackI()
    {
    }

    public void
    initiateCallback(CallbackReceiverPrx proxy, Ice.Current current)
    {
        proxy.callback(current.ctx);
    }

    public void
    shutdown(Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }
}
