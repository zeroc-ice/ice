// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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
