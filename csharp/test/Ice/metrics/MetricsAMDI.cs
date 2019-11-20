//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Threading.Tasks;
using Test;

public sealed class ControllerI : Controller
{
    public ControllerI(Ice.ObjectAdapter adapter)
    {
        _adapter = adapter;
    }

    public void hold(Ice.Current current)
    {
        _adapter.hold();
        _adapter.waitForHold();
    }

    public void resume(Ice.Current current)
    {
        _adapter.activate();
    }

    readonly private Ice.ObjectAdapter _adapter;
};

public sealed class MetricsI : Metrics
{
    public Task opAsync(Ice.Current current)
    {
        return null;
    }

    public Task failAsync(Ice.Current current)
    {
        current.con.close(Ice.ConnectionClose.Forcefully);
        return null;
    }

    public Task opWithUserExceptionAsync(Ice.Current current)
    {
        throw new UserEx();
    }

    public Task
    opWithRequestFailedExceptionAsync(Ice.Current current)
    {
        throw new Ice.ObjectNotExistException();
    }

    public Task
    opWithLocalExceptionAsync(Ice.Current current)
    {
        throw new Ice.SyscallException();
    }

    public Task
    opWithUnknownExceptionAsync(Ice.Current current)
    {
        throw new ArgumentOutOfRangeException();
    }

    public Task
    opByteSAsync(byte[] bs, Ice.Current current)
    {
        return null;
    }

    public Ice.ObjectPrx
    getAdmin(Ice.Current current)
    {
        return current.adapter.getCommunicator().getAdmin();
    }

    public void
    shutdown(Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }
}
