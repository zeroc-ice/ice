// **********************************************************************
//
// Copyright (c) 2002
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

final class CallbackReceiverI extends CallbackReceiver
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
