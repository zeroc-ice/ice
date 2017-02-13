// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.metrics;
import test.Ice.metrics.AMD.Test.*;

public final class AMDMetricsI extends _MetricsDisp
{
    public
    AMDMetricsI()
    {
    }

    @Override
    public void
    op_async(AMD_Metrics_op cb, Ice.Current current)
    {
        cb.ice_response();
    }

    @Override
    public void
    fail_async(AMD_Metrics_fail cb, Ice.Current current)
    {
        current.con.close(true);
        cb.ice_response();
    }

    @Override
    public void
    opWithUserException_async(AMD_Metrics_opWithUserException cb, Ice.Current current)
        throws UserEx
    {
        cb.ice_exception(new UserEx());
    }

    @Override
    public void
    opWithRequestFailedException_async(AMD_Metrics_opWithRequestFailedException cb, Ice.Current current)
    {
        cb.ice_exception(new Ice.ObjectNotExistException());
    }

    @Override
    public void
    opWithLocalException_async(AMD_Metrics_opWithLocalException cb, Ice.Current current)
    {
        cb.ice_exception(new Ice.SyscallException());
    }

    @Override
    public void
    opWithUnknownException_async(AMD_Metrics_opWithUnknownException cb, Ice.Current current)
    {
        cb.ice_exception(new IllegalArgumentException());
    }

    @Override
    public void
    opByteS_async(AMD_Metrics_opByteS cb, byte[] bs, Ice.Current current)
    {
        cb.ice_response();
    }

    @Override
    public Ice.ObjectPrx
    getAdmin(Ice.Current current)
    {
        return current.adapter.getCommunicator().getAdmin();
    }

    @Override
    public void
    shutdown(Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }
}
