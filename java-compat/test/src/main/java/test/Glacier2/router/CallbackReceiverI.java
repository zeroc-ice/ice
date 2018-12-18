// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.Glacier2.router;

import test.Glacier2.router.Test.CallbackException;
import test.Glacier2.router.Test._CallbackReceiverDisp;

final class CallbackReceiverI extends _CallbackReceiverDisp
{
    CallbackReceiverI()
    {
        _callback = false;
    }

    public synchronized void
    callback(Ice.Current current)
    {
        assert(!_callback);
        _callback = true;
        notify();
    }

    public void
    callbackEx(Ice.Current current)
        throws CallbackException
    {
        callback(current);
        CallbackException ex = new CallbackException();
        ex.someValue = 3.14;
        ex.someString = "3.14";
        throw ex;
    }

    synchronized void
    callbackOK()
    {
        while(!_callback)
        {
            try
            {
                wait();
            }
            catch(InterruptedException ex)
            {
            }
        }

        _callback = false;
    }

    private boolean _callback;
}
