// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Test.*;

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
