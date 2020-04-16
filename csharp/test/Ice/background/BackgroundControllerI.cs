//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;
using System.Collections.Generic;

internal class BackgroundController : IBackgroundController
{
    public void pauseCall(string opName, Ice.Current current)
    {
        lock (this)
        {
            _pausedCalls.Add(opName);
        }
    }

    public void resumeCall(string opName, Ice.Current current)
    {
        lock (this)
        {
            _pausedCalls.Remove(opName);
            System.Threading.Monitor.PulseAll(this);
        }
    }

    internal void checkCallPause(Ice.Current current)
    {
        lock (this)
        {
            while (_pausedCalls.Contains(current.Operation))
            {
                System.Threading.Monitor.Wait(this);
                break;
            }
        }
    }

    public void initializeException(bool enable, Ice.Current current)
    {
        _configuration.InitializeException(enable ? new Ice.TransportException("") : null);
    }

    public void readReady(bool enable, Ice.Current current)
    {
        _configuration.ReadReady(enable);
    }

    public void readException(bool enable, Ice.Current current)
    {
        _configuration.ReadException(enable ? new Ice.TransportException("") : null);
    }

    public void writeReady(bool enable, Ice.Current current)
    {
        _configuration.WriteReady(enable);
    }

    public void writeException(bool enable, Ice.Current current)
    {
        _configuration.WriteException(enable ? new Ice.TransportException("") : null);
    }

    public void buffered(bool enable, Ice.Current current)
    {
        _configuration.Buffered(enable);
    }

    internal BackgroundController(Ice.ObjectAdapter adapter)
    {
        _adapter = adapter;
        _configuration = Configuration.GetInstance();
    }
    private Ice.ObjectAdapter _adapter;
    private Configuration _configuration;
    private HashSet<string> _pausedCalls = new HashSet<string>();
}
