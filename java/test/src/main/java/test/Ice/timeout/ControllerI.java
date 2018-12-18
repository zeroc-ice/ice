// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.timeout;

import test.Ice.timeout.Test.Controller;

class ControllerI implements Controller
{
    static class ActivateAdapterThread extends Thread
    {
        ActivateAdapterThread(com.zeroc.Ice.ObjectAdapter adapter, int timeout)
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

        com.zeroc.Ice.ObjectAdapter _adapter;
        int _timeout;
    }

    public ControllerI(com.zeroc.Ice.ObjectAdapter adapter)
    {
        _adapter = adapter;
    }

    @Override
    public void holdAdapter(int to, com.zeroc.Ice.Current current)
    {
        _adapter.hold();
        if(to >= 0)
        {
            Thread thread = new ActivateAdapterThread(_adapter, to);
            thread.start();
        }
    }

    @Override
    public void resumeAdapter(com.zeroc.Ice.Current current)
    {
        _adapter.activate();
    }

    @Override
    public void shutdown(com.zeroc.Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }

    final com.zeroc.Ice.ObjectAdapter _adapter;
}
