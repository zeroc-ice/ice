// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
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

    public void
    op(Ice.Current current)
    {
    }

    public void
    fail(Ice.Current current)
    {
        current.con.close(true);
    }

    public void
    opWithUserException(Ice.Current current)
        throws UserEx
    {
        throw new UserEx();
    }

    public void
    opWithRequestFailedException(Ice.Current current)
    {
        throw new Ice.ObjectNotExistException();
    }

    public void
    opWithLocalException(Ice.Current current)
    {
        throw new Ice.SyscallException();
    }

    public void
    opWithUnknownException(Ice.Current current)
    {
        throw new IllegalArgumentException();
    }

    public void
    opByteS(byte[] bs, Ice.Current current)
    {
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
