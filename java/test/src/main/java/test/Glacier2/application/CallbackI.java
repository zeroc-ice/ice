//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Glacier2.application;

import test.Glacier2.application.Test.CallbackReceiverPrx;
import test.Glacier2.application.Test.Callback;

final class CallbackI implements Callback
{
    CallbackI()
    {
    }

    public void initiateCallback(CallbackReceiverPrx proxy, com.zeroc.Ice.Current current)
    {
        proxy.callback(current.ctx);
    }

    public void shutdown(com.zeroc.Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }
}
