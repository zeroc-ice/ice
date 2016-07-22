// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
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
    override public void opAsync(Action response, Action<Exception> exception, Ice.Current current)
    {
        response();
    }

    override public void failAsync(Action response, Action<Exception> exception, Ice.Current current)
    { 
        current.con.close(true);
        response();
    }

    override public void opWithUserExceptionAsync(Action response, Action<Exception> exception, Ice.Current current)
    {
        exception(new UserEx());
    }

    override public void
    opWithRequestFailedExceptionAsync(Action response, Action<Exception> exception, Ice.Current current)
    {
        exception(new Ice.ObjectNotExistException());
    }

    override public void opWithLocalExceptionAsync(Action response, Action<Exception> exception, Ice.Current current)
    {
        exception(new Ice.SyscallException());
    }

    override public void opWithUnknownExceptionAsync(Action response, Action<Exception> exception, Ice.Current current)
    {
        exception(new ArgumentOutOfRangeException());
    }

    override public void opByteSAsync(byte[] bs, Action response, Action<Exception> exception, Ice.Current current)
    {
        response();
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
