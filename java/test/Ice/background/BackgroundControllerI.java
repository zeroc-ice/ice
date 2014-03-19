// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package test.Ice.background;

import test.Ice.background.Test._BackgroundControllerDisp;

class BackgroundControllerI extends _BackgroundControllerDisp
{
    synchronized public void
    pauseCall(String opName, Ice.Current current)
    {
        _pausedCalls.add(opName);
    }

    synchronized public void
    resumeCall(String opName, Ice.Current current)
    {
        _pausedCalls.remove(opName);
        notifyAll();
    }

    synchronized public void
    checkCallPause(Ice.Current current)
    {
        while(_pausedCalls.contains(current.operation))
        {
            try
            {
                wait();
                break;
            }
            catch(java.lang.InterruptedException ex)
            {
            }
        }
    }

    public void
    holdAdapter(Ice.Current current)
    {
        _adapter.hold();
    }
    
    public void
    resumeAdapter(Ice.Current current)
    {
        _adapter.activate();
    }
    
    public void
    initializeSocketStatus(int status, Ice.Current current)
    {
        _configuration.initializeSocketStatus(status);
    }

    public void
    initializeException(boolean enable, Ice.Current current)
    {
        _configuration.initializeException(enable ? new Ice.SocketException() : null);
    }

    public void
    readReady(boolean enable, Ice.Current current)
    {
        _configuration.readReady(enable);
    }

    public void
    readException(boolean enable, Ice.Current current)
    {
        _configuration.readException(enable ? new Ice.SocketException() : null);
    }
    
    public void
    writeReady(boolean enable, Ice.Current current)
    {
        _configuration.writeReady(enable);
    }

    public void
    writeException(boolean enable, Ice.Current current)
    {
        _configuration.writeException(enable ? new Ice.SocketException() : null);
    }
    
    public
    BackgroundControllerI(Configuration configuration, Ice.ObjectAdapter adapter)
    {
        _adapter = adapter;
        _configuration = configuration;
    }

    final private Ice.ObjectAdapter _adapter;
    final private java.util.Set<String> _pausedCalls = new java.util.HashSet<String>();
    final private Configuration _configuration;
}
