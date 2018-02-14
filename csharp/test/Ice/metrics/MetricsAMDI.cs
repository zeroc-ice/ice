// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
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
    override public void op_async(Test.AMD_Metrics_op cb, Ice.Current current)
    {
        cb.ice_response();
    }

    override public void fail_async(Test.AMD_Metrics_fail cb, Ice.Current current)
    { 
        current.con.close(true);
        cb.ice_response();
    }

    override public void opWithUserException_async(Test.AMD_Metrics_opWithUserException cb, Ice.Current current)
    {
        cb.ice_exception(new UserEx());
    }

    override public void opWithRequestFailedException_async(Test.AMD_Metrics_opWithRequestFailedException cb,
                                                            Ice.Current current)
    {
        cb.ice_exception(new Ice.ObjectNotExistException());
    }

    override public void opWithLocalException_async(Test.AMD_Metrics_opWithLocalException cb, Ice.Current current)
    {
        cb.ice_exception(new Ice.SyscallException());
    }

    override public void opWithUnknownException_async(Test.AMD_Metrics_opWithUnknownException cb, Ice.Current current)
    {
        cb.ice_exception(new ArgumentOutOfRangeException());
    }

    override public void opByteS_async(Test.AMD_Metrics_opByteS cb, byte[] bs, Ice.Current current)
    {
        cb.ice_response();
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
