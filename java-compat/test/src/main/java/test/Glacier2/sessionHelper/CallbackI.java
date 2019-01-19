//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Glacier2.sessionHelper;

import test.Glacier2.sessionHelper.Test.CallbackException;
import test.Glacier2.sessionHelper.Test.CallbackReceiverPrx;
import test.Glacier2.sessionHelper.Test._CallbackDisp;

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
    initiateCallbackEx(CallbackReceiverPrx proxy, Ice.Current current)
        throws CallbackException
    {
        proxy.callbackEx(current.ctx);
    }

    public void
    shutdown(Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }
}
