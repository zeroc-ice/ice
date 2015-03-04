// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
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

    public void
    op_async(AMD_Metrics_op cb, Ice.Current current)
    {
        cb.ice_response();
    }

    public void
    fail_async(AMD_Metrics_fail cb, Ice.Current current)
    {
        current.con.close(true);
        cb.ice_response();
    }

    public void
    opWithUserException_async(AMD_Metrics_opWithUserException cb, Ice.Current current)
        throws UserEx
    {
        cb.ice_exception(new UserEx());
    }

    public void
    opWithRequestFailedException_async(AMD_Metrics_opWithRequestFailedException cb, Ice.Current current)
    {
        cb.ice_exception(new Ice.ObjectNotExistException());
    }

    public void
    opWithLocalException_async(AMD_Metrics_opWithLocalException cb, Ice.Current current)
    {
        cb.ice_exception(new Ice.SyscallException());
    }

    public void
    opWithUnknownException_async(AMD_Metrics_opWithUnknownException cb, Ice.Current current)
    {
        cb.ice_exception(new IllegalArgumentException());
    }

    public void
    opByteS_async(AMD_Metrics_opByteS cb, byte[] bs, Ice.Current current)
    {
        cb.ice_response();
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
