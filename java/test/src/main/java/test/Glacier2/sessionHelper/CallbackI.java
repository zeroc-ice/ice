//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Glacier2.sessionHelper;

import test.Glacier2.sessionHelper.Test.CallbackException;
import test.Glacier2.sessionHelper.Test.CallbackReceiverPrx;
import test.Glacier2.sessionHelper.Test.Callback;

final class CallbackI implements Callback
{
    CallbackI()
    {
    }

    public void initiateCallback(CallbackReceiverPrx proxy, com.zeroc.Ice.Current current)
    {
        proxy.callback(current.ctx);
    }

    public void initiateCallbackEx(CallbackReceiverPrx proxy, com.zeroc.Ice.Current current)
        throws CallbackException
    {
        proxy.callbackEx(current.ctx);
    }

    public void shutdown(com.zeroc.Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }
}
