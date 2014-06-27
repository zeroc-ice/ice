// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;

public sealed class RetryI : Test.RetryDisp_
{
    public RetryI()
    {
    }

    public override void op(bool kill, Ice.Current current)
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

    public override int opIdempotent(int counter, Ice.Current current)
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
    
    public override void opNotIdempotent(int counter, Ice.Current current)
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
    
    public override void opSystemException(Ice.Current c)
    {
        throw new SystemFailure();
    }
    
    public override void shutdown(Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }

    private int _counter;
    static readonly int nRetry = 4;
}
