// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.metrics;
import test.Ice.metrics.Test.*;

public final class MetricsI extends _MetricsDisp
{
    public
    MetricsI()
    {
    }

    @Override
    public void
    op(Ice.Current current)
    {
    }

    @Override
    public void
    fail(Ice.Current current)
    {
        current.con.close(true);
    }

    @Override
    public void
    opWithUserException(Ice.Current current)
        throws UserEx
    {
        throw new UserEx();
    }

    @Override
    public void
    opWithRequestFailedException(Ice.Current current)
    {
        throw new Ice.ObjectNotExistException();
    }

    @Override
    public void
    opWithLocalException(Ice.Current current)
    {
        throw new Ice.SyscallException();
    }

    @Override
    public void
    opWithUnknownException(Ice.Current current)
    {
        throw new IllegalArgumentException();
    }

    @Override
    public void
    opByteS(byte[] bs, Ice.Current current)
    {
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
