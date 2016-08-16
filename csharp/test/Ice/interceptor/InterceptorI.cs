// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Threading.Tasks;

class InterceptorI : Ice.DispatchInterceptor
{
    internal InterceptorI(Ice.Object servant)
    {
        servant_ = servant;
    }

    protected static void
    test(bool b)
    {
        if(!b)
        {
            throw new Exception();
        }
    }

    public override Task<Ice.OutputStream>
    dispatch(Ice.Request request)
    {
        Ice.Current current = request.getCurrent();
        lastOperation_ = current.operation;

        if(lastOperation_.Equals("addWithRetry") || lastOperation_.Equals("amdAddWithRetry"))
        {
            for(int i = 0; i < 10; ++i)
            {
                try
                {
                    servant_.ice_dispatch(request);
                    test(false);
                }
                catch(Test.RetryException)
                {
                    //
                    // Expected, retry
                    //
                }
            }

            current.ctx["retry"] = "no";
        }
        var task = servant_.ice_dispatch(request);
        lastStatus_ = task != null;
        return task;
    }

    internal bool
    getLastStatus()
    {
        return lastStatus_;
    }

    internal String
    getLastOperation()
    {
        return lastOperation_;
    }

    internal virtual void
    clear()
    {
        lastOperation_ = null;
        lastStatus_ = false;
    }

    protected readonly Ice.Object servant_;
    protected string lastOperation_;
    protected bool lastStatus_ = false;
}
