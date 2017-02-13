// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.metrics;

import test.Ice.metrics.Test.*;

public final class MetricsI implements Metrics
{
    public MetricsI()
    {
    }

    @Override
    public void op(com.zeroc.Ice.Current current)
    {
    }

    @Override
    public void fail(com.zeroc.Ice.Current current)
    {
        current.con.close(com.zeroc.Ice.ConnectionClose.Forcefully);
    }

    @Override
    public void opWithUserException(com.zeroc.Ice.Current current)
        throws UserEx
    {
        throw new UserEx();
    }

    @Override
    public void opWithRequestFailedException(com.zeroc.Ice.Current current)
    {
        throw new com.zeroc.Ice.ObjectNotExistException();
    }

    @Override
    public void opWithLocalException(com.zeroc.Ice.Current current)
    {
        throw new com.zeroc.Ice.SyscallException();
    }

    @Override
    public void opWithUnknownException(com.zeroc.Ice.Current current)
    {
        throw new IllegalArgumentException();
    }

    @Override
    public void opByteS(byte[] bs, com.zeroc.Ice.Current current)
    {
    }

    @Override
    public com.zeroc.Ice.ObjectPrx getAdmin(com.zeroc.Ice.Current current)
    {
        return current.adapter.getCommunicator().getAdmin();
    }

    @Override
    public void shutdown(com.zeroc.Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }
}
