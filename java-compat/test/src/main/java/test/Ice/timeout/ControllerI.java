// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.timeout;

import test.Ice.timeout.Test._ControllerDisp;

class ControllerI extends _ControllerDisp
{
    static class ActivateAdapterThread extends Thread
    {
        ActivateAdapterThread(Ice.ObjectAdapter adapter, int timeout)
        {
            _adapter = adapter;
            _timeout = timeout;
        }

        @Override
        public void run()
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

    public ControllerI(Ice.ObjectAdapter adapter)
    {
        _adapter = adapter;
    }

    @Override
    public void holdAdapter(int to, Ice.Current current)
    {
        _adapter.hold();
        if(to >= 0)
        {
            Thread thread = new ActivateAdapterThread(_adapter, to);
            thread.start();
        }
    }

    @Override
    public void resumeAdapter(Ice.Current current)
    {
        _adapter.activate();
    }

    @Override
    public void shutdown(Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }

    final Ice.ObjectAdapter _adapter;
}
