// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.retry;

import test.Ice.retry.Test.Retry;

public final class RetryI implements Retry
{
    public RetryI()
    {
    }

    @Override
    public void op(boolean kill, com.zeroc.Ice.Current current)
    {
        if(kill)
        {
            if(current.con != null)
            {
                current.con.close(com.zeroc.Ice.ConnectionClose.Forcefully);
            }
            else
            {
                throw new com.zeroc.Ice.ConnectionLostException();
            }
        }
    }

    @Override
    public int opIdempotent(int nRetry, com.zeroc.Ice.Current current)
    {
        if(nRetry < 0)
        {
            _counter = 0;
            return 0;
        }

        if(nRetry > _counter)
        {
            ++_counter;
            throw new com.zeroc.Ice.ConnectionLostException();
        }

        int counter = _counter;
        _counter = 0;
        return counter;
    }

    @Override
    public void opNotIdempotent(com.zeroc.Ice.Current current)
    {
        throw new com.zeroc.Ice.ConnectionLostException();
    }

    @Override
    public void opSystemException(com.zeroc.Ice.Current c)
    {
        throw new SystemFailure();
    }

    @Override
    public void shutdown(com.zeroc.Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }

    private int _counter;
}
