// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Test.*;

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

    synchronized boolean
    callbackOK()
    {
        while(!_callback)
        {
            try
            {
                wait(5000);
                if(!_callback)
                {
                    return false;
                }
            }
            catch(InterruptedException ex)
            {
            }
        }

        _callback = false;
        return true;
    }

    private boolean _callback;
}
