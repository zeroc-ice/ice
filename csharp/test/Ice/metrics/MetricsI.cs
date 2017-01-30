// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Collections.Generic;
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
    override public void op(Ice.Current current)
    {
    }

    override public void fail(Ice.Current current)
    {
        current.con.close(Ice.ConnectionClose.CloseForcefully);
    }

    override public void opWithUserException(Ice.Current current)
    {
        throw new UserEx();
    }

    override public void opWithRequestFailedException(Ice.Current current)
    {
        throw new Ice.ObjectNotExistException();
    }

    override public void opWithLocalException(Ice.Current current)
    {
        throw new Ice.SyscallException();
    }

    override public void opWithUnknownException(Ice.Current current)
    {
        throw new ArgumentOutOfRangeException();
    }

    override public void opByteS(byte[] bs, Ice.Current current)
    {
    }

    override public Ice.ObjectPrx getAdmin(Ice.Current current)
    {
        return current.adapter.getCommunicator().getAdmin();
    }

    override public void shutdown(Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }
}
