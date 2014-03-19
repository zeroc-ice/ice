// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Test;
using System.Threading;
using System.Collections.Generic;

internal class BackgroundControllerI : BackgroundControllerDisp_
{
    public override void pauseCall(string opName, Ice.Current current)
    {
        _m.Lock();
        try
        {
            _pausedCalls.Add(opName);
        }
        finally
        {
            _m.Unlock();
        }
    }

    public override void resumeCall(string opName, Ice.Current current)
    {
        _m.Lock();
        try
        {
            _pausedCalls.Remove(opName);
            _m.NotifyAll();
        }
        finally
        {
            _m.Unlock();
        }
    }

    internal void checkCallPause(Ice.Current current)
    {
        _m.Lock();
        try
        {
            while(_pausedCalls.Contains(current.operation))
            {
                _m.Wait();
                break;
            }
        }
        finally
        {
            _m.Unlock();
        }
    }

    public override void holdAdapter(Ice.Current current)
    {
        _adapter.hold();
    }

    public override void resumeAdapter(Ice.Current current)
    {
        _adapter.activate();
    }

    public override void initializeException(bool enable, Ice.Current current)
    {
        _configuration.initializeException(enable ? new Ice.SocketException() : null);
    }

    public override void readReady(bool enable, Ice.Current current)
    {
        _configuration.readReady(enable);
    }

    public override void readException(bool enable, Ice.Current current)
    {
        _configuration.readException(enable ? new Ice.SocketException() : null);
    }

    public override void writeReady(bool enable, Ice.Current current)
    {
        _configuration.writeReady(enable);
    }

    public override void writeException(bool enable, Ice.Current current)
    {
        _configuration.writeException(enable ? new Ice.SocketException() : null);
    }

    internal BackgroundControllerI(Ice.ObjectAdapter adapter)
    {
        _adapter = adapter;
        _configuration = Configuration.getInstance();
    }

    private Ice.ObjectAdapter _adapter;
#if COMPACT
    private IceInternal.HashSet<string> _pausedCalls = new IceInternal.HashSet<string>();
#else
    private HashSet<string> _pausedCalls = new HashSet<string>();
#endif
    private Configuration _configuration;
    private readonly IceUtilInternal.Monitor _m = new IceUtilInternal.Monitor();
}
