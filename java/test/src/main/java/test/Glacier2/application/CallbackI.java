// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

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
