// Copyright (c) ZeroC, Inc.

using System.Diagnostics;
using Test;

namespace Glacier2.router;

public sealed class CallbackI : Test.CallbackDisp_
{
    public CallbackI()
    {
    }

    public override void
    initiateCallback(CallbackReceiverPrx proxy, Ice.Current current) => proxy.callback(current.ctx);

    public override void
    initiateCallbackEx(CallbackReceiverPrx proxy, Ice.Current current) => proxy.callbackEx(current.ctx);

    public override void
    shutdown(Ice.Current current) => current.adapter.getCommunicator().shutdown();
}

public sealed class CallbackReceiverI : CallbackReceiverDisp_
{
    public CallbackReceiverI() => _callback = false;

    public override void
    callback(Ice.Current current)
    {
        lock (_mutex)
        {
            Debug.Assert(!_callback);
            _callback = true;
            Monitor.Pulse(_mutex);
        }
    }

    public override void
    callbackEx(Ice.Current current)
    {
        callback(current);
        var ex = new CallbackException();
        ex.someValue = 3.14;
        ex.someString = "3.14";
        throw ex;
    }

    public void
    callbackOK()
    {
        lock (_mutex)
        {
            while (!_callback)
            {
                Monitor.Wait(_mutex, 30000);
                TestHelper.test(_callback);
            }

            _callback = false;
        }
    }

    private bool _callback;
    private readonly object _mutex = new();
}
