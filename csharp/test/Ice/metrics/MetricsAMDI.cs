//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Diagnostics;
using System.Threading.Tasks;
using Test;

public sealed class Controller : IController
{
    public Controller(Ice.ObjectAdapter adapter) => _adapter = adapter;

    public void hold(Ice.Current current)
    {
        _adapter.Hold();
        _adapter.WaitForHold();
    }

    public void resume(Ice.Current current) => _adapter.Activate();

    readonly private Ice.ObjectAdapter _adapter;
};

public sealed class Metrics : IMetrics
{
    public ValueTask opAsync(Ice.Current current) => new ValueTask(Task.CompletedTask);

    public ValueTask failAsync(Ice.Current current)
    {
        Debug.Assert(current != null);
        current.Connection.Close(Ice.ConnectionClose.Forcefully);
        return new ValueTask(Task.CompletedTask);
    }

    public ValueTask opWithUserExceptionAsync(Ice.Current current) => throw new UserEx();

    public ValueTask
    opWithRequestFailedExceptionAsync(Ice.Current current) => throw new Ice.ObjectNotExistException();

    public ValueTask
    opWithLocalExceptionAsync(Ice.Current current) => throw new Ice.InvalidConfigurationException("fake");

    public ValueTask
    opWithUnknownExceptionAsync(Ice.Current current) => throw new ArgumentOutOfRangeException();

    public ValueTask
    opByteSAsync(byte[] bs, Ice.Current current) => new ValueTask(Task.CompletedTask);

    public Ice.IObjectPrx
    getAdmin(Ice.Current current)
    {
        Debug.Assert(current != null);
        return current.Adapter.Communicator.GetAdmin();
    }

    public void
    shutdown(Ice.Current current) => current.Adapter.Communicator.Shutdown();
}
