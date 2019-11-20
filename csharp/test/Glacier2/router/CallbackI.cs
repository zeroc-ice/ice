//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;
using System.Diagnostics;

public sealed class CallbackI : Test.Callback
{
    public CallbackI()
    {
    }

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
        current.adapter.GetCommunicator().shutdown();
    }
}

public sealed class CallbackReceiverI : CallbackReceiver
{
    public CallbackReceiverI()
    {
        _callback = false;
    }

    public void
    callback(Ice.Current current)
    {
        lock (this)
        {
            Debug.Assert(!_callback);
            _callback = true;
            System.Threading.Monitor.Pulse(this);
        }
    }

    public void
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
        lock (this)
        {
            while (!_callback)
            {
                System.Threading.Monitor.Wait(this, 30000);
                TestHelper.test(_callback);
            }

            _callback = false;
        }
    }

    private bool _callback;
}
