// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.background;

import test.Ice.background.Test.BackgroundController;

class BackgroundControllerI implements BackgroundController
{
    @Override
    synchronized public void pauseCall(String opName, com.zeroc.Ice.Current current)
    {
        _pausedCalls.add(opName);
    }

    @Override
    synchronized public void resumeCall(String opName, com.zeroc.Ice.Current current)
    {
        _pausedCalls.remove(opName);
        notifyAll();
    }

    synchronized public void checkCallPause(com.zeroc.Ice.Current current)
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

    @Override
    public void holdAdapter(com.zeroc.Ice.Current current)
    {
        _adapter.hold();
    }
    
    @Override
    public void resumeAdapter(com.zeroc.Ice.Current current)
    {
        _adapter.activate();
    }
    
    @Override
    public void initializeSocketStatus(int status, com.zeroc.Ice.Current current)
    {
        _configuration.initializeSocketStatus(status);
    }

    @Override
    public void initializeException(boolean enable, com.zeroc.Ice.Current current)
    {
        _configuration.initializeException(enable ? new com.zeroc.Ice.SocketException() : null);
    }

    @Override
    public void readReady(boolean enable, com.zeroc.Ice.Current current)
    {
        _configuration.readReady(enable);
    }

    @Override
    public void readException(boolean enable, com.zeroc.Ice.Current current)
    {
        _configuration.readException(enable ? new com.zeroc.Ice.SocketException() : null);
    }
    
    @Override
    public void writeReady(boolean enable, com.zeroc.Ice.Current current)
    {
        _configuration.writeReady(enable);
    }

    @Override
    public void writeException(boolean enable, com.zeroc.Ice.Current current)
    {
        _configuration.writeException(enable ? new com.zeroc.Ice.SocketException() : null);
    }

    @Override
    public void buffered(boolean enable, com.zeroc.Ice.Current current)
    {
        _configuration.buffered(enable);
    }
    
    public BackgroundControllerI(Configuration configuration, com.zeroc.Ice.ObjectAdapter adapter)
    {
        _adapter = adapter;
        _configuration = configuration;
    }

    final private com.zeroc.Ice.ObjectAdapter _adapter;
    final private java.util.Set<String> _pausedCalls = new java.util.HashSet<>();
    final private Configuration _configuration;
}
