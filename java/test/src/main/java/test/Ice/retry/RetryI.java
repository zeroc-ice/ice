// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.retry;
import test.Ice.retry.Test._RetryDisp;

public final class RetryI extends _RetryDisp
{
    public
    RetryI()
    {
    }

    @Override
    public void
    op(boolean kill, Ice.Current current)
    {
        if(kill)
        {
            if(current.con != null)
            {
                current.con.close(true);
            }
            else
            {
                throw new Ice.ConnectionLostException();
            }
        }
    }

    @Override
    public int
    opIdempotent(int nRetry, Ice.Current current)
    {
        if(nRetry < 0)
        {
            _counter = 0;
            return 0;
        }

        if(nRetry > _counter)
        {
            ++_counter;
            throw new Ice.ConnectionLostException();
        }

        int counter = _counter;
        _counter = 0;
        return counter;
    }
    
    @Override
    public void
    opNotIdempotent(Ice.Current current)
    {
        throw new Ice.ConnectionLostException();
    }
    
    @Override
    public void
    opSystemException(Ice.Current c)
    {
        throw new SystemFailure();
    }
    
    @Override
    public void
    shutdown(Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }

    private int _counter;
}
