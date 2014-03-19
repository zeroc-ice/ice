// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.timeout;
import test.Ice.timeout.Test._TimeoutDisp;


class TimeoutI extends _TimeoutDisp
{
    static class ActivateAdapterThread extends Thread
    {
        ActivateAdapterThread(Ice.ObjectAdapter adapter, int timeout)
        {
            _adapter = adapter;
            _timeout = timeout;
        }

        public void
        run()
        {
            _adapter.waitForHold();
            try
            {
                sleep(_timeout);
            }
            catch(InterruptedException ex)
            {
            }
            _adapter.activate();
        }

        Ice.ObjectAdapter _adapter;
        int _timeout;
    }

    public void
    op(Ice.Current current)
    {
    }

    public void
    sendData(byte[] seq, Ice.Current current)
    {
    }

    public void
    sleep(int to, Ice.Current current)
    {
        try
        {
            Thread.currentThread().sleep(to);
        }
        catch(InterruptedException ex)
        {
        }
    }

    public void
    holdAdapter(int to, Ice.Current current)
    {
        current.adapter.hold();
        Thread thread = new ActivateAdapterThread(current.adapter, to);
        thread.start();
    }

    public void
    shutdown(Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }
}
