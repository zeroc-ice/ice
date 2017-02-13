// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Threading;

class MySystemException : Ice.SystemException
{
    public 
    MySystemException()
    {
    }

    override public string 
    ice_name()
    {
        return "MySystemException";
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

    public override void 
    amdAdd_async(Test.AMD_MyObject_amdAdd cb, int x, int y, Ice.Current current)
    {
        Thread thread = new Thread(delegate()
                                   {
                                       Thread.Sleep(10);
                                       cb.ice_response(x + y);
                                   });
        thread.IsBackground = true;
        thread.Start();
    }

    public override void 
    amdAddWithRetry_async(Test.AMD_MyObject_amdAddWithRetry cb, int x, int y, Ice.Current current)
    {
        Thread thread = new Thread(delegate()
                                   {
                                       Thread.Sleep(10);
                                       cb.ice_response(x + y);
                                   });
        thread.IsBackground = true;
        thread.Start();

        if(current.ctx.ContainsKey("retry") && current.ctx["retry"].Equals("no"))
        {
            return;
        }
        else
        {
            throw new Test.RetryException();
        }
    } 
    
    public override void 
    amdBadAdd_async(Test.AMD_MyObject_amdBadAdd cb, int x, int y, Ice.Current current)
    {
        Thread thread = new Thread(delegate()
                                   {
                                       Thread.Sleep(10);
                                       cb.ice_exception(new Test.InvalidInputException());
                                   });
        thread.IsBackground = true;
        thread.Start();
    } 

    public override void 
    amdNotExistAdd_async(Test.AMD_MyObject_amdNotExistAdd cb, int x, int y, Ice.Current current)
    {
        Thread thread = new Thread(delegate()
                                   {
                                       Thread.Sleep(10);
                                       cb.ice_exception(new Ice.ObjectNotExistException());
                                   });
        thread.IsBackground = true;
        thread.Start();
    } 
    
    public override void 
    amdBadSystemAdd_async(Test.AMD_MyObject_amdBadSystemAdd cb, int x, int y, Ice.Current current)
    {
        Thread thread = new Thread(delegate()
                                   {
                                       Thread.Sleep(10);
                                       cb.ice_exception(new MySystemException());
                                   });
        
        thread.IsBackground = true;
        thread.Start();
    } 
}
