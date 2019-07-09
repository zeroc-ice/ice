//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
        lock(this)
        {
            Debug.Assert(!_callback);
            _callback = true;
            System.Threading.Monitor.Pulse(this);
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
        lock(this)
        {
            while(!_callback)
            {
                System.Threading.Monitor.Wait(this, 30000);
                TestHelper.test(_callback);
            }

            _callback = false;
        }
    }

    private bool _callback;
}
