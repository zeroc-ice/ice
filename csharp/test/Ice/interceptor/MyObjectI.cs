// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Threading.Tasks;

class MySystemException : Ice.SystemException
{
    public
    MySystemException()
    {
    }

    override public string
    ice_id()
    {
        return "::MySystemException";
    }
};

class MyObjectI : Test.MyObjectDisp_
{
    protected static void
    test(bool b)
    {
        if(!b)
        {
            throw new Exception();
        }
    }

    public override int
    add(int x, int y, Ice.Current current)
    {
        return x + y;
    }

    public override int
    addWithRetry(int x, int y, Ice.Current current)
    {
        test(current != null);
        test(current.ctx != null);

        if(current.ctx.ContainsKey("retry") && current.ctx["retry"].Equals("no"))
        {
            return x + y;
        }
        throw new Test.RetryException();
    }

    public override int
    badAdd(int x, int y, Ice.Current current)
    {
        throw new Test.InvalidInputException();
    }

    public override int
    notExistAdd(int x, int y, Ice.Current current)
    {
        throw new Ice.ObjectNotExistException();
    }

    public override int
    badSystemAdd(int x, int y, Ice.Current current)
    {
        throw new MySystemException();
    }

    //
    // AMD
    //
    public override async Task<int>
    amdAddAsync(int x, int y, Ice.Current current)
    {
        await Task.Delay(10);
        return x + y;
    }

    public override async Task<int>
    amdAddWithRetryAsync(int x, int y, Ice.Current current)
    {
        if(current.ctx.ContainsKey("retry") && current.ctx["retry"].Equals("no"))
        {
            await Task.Delay(10);
            return x + y;
        }
        else
        {
            throw new Test.RetryException();
        }
    }

    public override async Task<int>
    amdBadAddAsync(int x, int y, Ice.Current current)
    {
        await Task.Delay(10);
        throw new Test.InvalidInputException();
    }

    public override async Task<int>
    amdNotExistAddAsync(int x, int y, Ice.Current current)
    {
        await Task.Delay(10);
        throw new Ice.ObjectNotExistException();
    }

    public override async Task<int>
    amdBadSystemAddAsync(int x, int y, Ice.Current current)
    {
        await Task.Delay(10);
        throw new MySystemException();
    }
}
