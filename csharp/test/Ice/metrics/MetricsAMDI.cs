// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Threading.Tasks;
using Test;

public sealed class ControllerI : ControllerDisp_
{
    public ControllerI(Ice.ObjectAdapter adapter)
    {
        _adapter = adapter;
    }

    override public void hold(Ice.Current current)
    {
        _adapter.hold();
        _adapter.waitForHold();
    }

    override public void resume(Ice.Current current)
    {
        _adapter.activate();
    }

    readonly private Ice.ObjectAdapter _adapter;
};

public sealed class MetricsI : MetricsDisp_
{
    override public Task opAsync(Ice.Current current)
    {
        return null;
    }

    override public Task failAsync(Ice.Current current)
    {
        current.con.close(Ice.ConnectionClose.Forcefully);
        return null;
    }

    override public Task opWithUserExceptionAsync(Ice.Current current)
    {
        throw new UserEx();
    }

    override public Task
    opWithRequestFailedExceptionAsync(Ice.Current current)
    {
        throw new Ice.ObjectNotExistException();
    }

    override public Task
    opWithLocalExceptionAsync(Ice.Current current)
    {
        throw new Ice.SyscallException();
    }

    override public Task
    opWithUnknownExceptionAsync(Ice.Current current)
    {
        throw new ArgumentOutOfRangeException();
    }

    override public Task
    opByteSAsync(byte[] bs, Ice.Current current)
    {
        return null;
    }

    override public Ice.ObjectPrx
    getAdmin(Ice.Current current)
    {
        return current.adapter.getCommunicator().getAdmin();
    }

    override public void
    shutdown(Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }
}
