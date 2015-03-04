// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Test;
using System.Diagnostics;

public sealed class CallbackI : Test.CallbackDisp_
{
    public CallbackI()
    {
    }

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

public sealed class CallbackReceiverI : CallbackReceiverDisp_
{
    public CallbackReceiverI()
    {
        _callback = false;
    }

    public override void
    callback(Ice.Current current)
    {
        _m.Lock();
        try
        {
            Debug.Assert(!_callback);
            _callback = true;
            _m.Notify();
        }
        finally
        {
            _m.Unlock();
        }
    }

    public override void
    callbackEx(Ice.Current current)
    {
        callback(current);
        CallbackException ex = new CallbackException();
        ex.someValue = 3.14;
        ex.someString = "3.14";
        throw ex;
    }

    public void
    callbackOK()
    {
        _m.Lock();
        try
        {
            while(!_callback)
            {
                _m.Wait();
            }
            
            _callback = false;
        }
        finally
        {
            _m.Unlock();
        }
    }

    private bool _callback;
    private readonly IceUtilInternal.Monitor _m = new IceUtilInternal.Monitor();
}
