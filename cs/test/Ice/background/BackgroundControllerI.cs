// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Test;
using System.Threading;

internal class BackgroundControllerI : BackgroundControllerDisp_
{
    public override void pauseCall(string opName, Ice.Current current)
    {
        lock(this)
        {
            _pausedCalls.Add(opName);
        }
    }

    public override void resumeCall(string opName, Ice.Current current)
    {
        lock(this)
        {
            _pausedCalls.Remove(opName);
            Monitor.PulseAll(this);
        }
    }

    internal void checkCallPause(Ice.Current current)
    {
        lock(this)
        {
            while(_pausedCalls.Contains(current.operation))
            {
                Monitor.Wait(this);
                break;
            }
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

    public override void initializeSocketStatus(int status, Ice.Current current)
    {
        _configuration.initializeSocketStatus((IceInternal.SocketStatus)status);
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
    private IceUtil.Set _pausedCalls = new IceUtil.Set();
    private Configuration _configuration;
}
