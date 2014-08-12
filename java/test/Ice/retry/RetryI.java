// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
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
    opIdempotent(int counter, Ice.Current current)
    {
        if(counter + nRetry > _counter)
        {
            ++_counter;
            if(current.con != null)
            {
                current.con.close(true);
            }
            else
            {
                throw new Ice.ConnectionLostException();
            }
        }
        return _counter;
    }
    
    @Override
    public void
    opNotIdempotent(int counter, Ice.Current current)
    {
        if(_counter != counter)
        {
            return;
        }
        
        ++_counter;
        if(current.con != null)
        {
            current.con.close(true);
        }
        else
        {
            throw new Ice.ConnectionLostException();
        }
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
    static final int nRetry = 4;
}
