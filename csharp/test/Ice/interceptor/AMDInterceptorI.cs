// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

//
// A dispatch interceptor with special handling for AMD requests
//

using System;

class AMDInterceptorI : InterceptorI, Ice.DispatchInterceptorAsyncCallback
{
    class DispatchInterceptorAsyncCallbackI : Ice.DispatchInterceptorAsyncCallback
    {
        public bool response(bool ok)
        {
            test(ok);
            return false;
        }
        
        public bool exception(System.Exception ex)
        {
            test(ex is Test.RetryException);
            return false;
        }
    };


    internal AMDInterceptorI(Ice.Object servant) : base(servant)
    {
    }
    
    public override Ice.DispatchStatus
    dispatch(Ice.Request request)
    {
        Ice.Current current = request.getCurrent();
        lastOperation_ = current.operation;

        if(lastOperation_.Equals("amdAddWithRetry"))
        {
            for(int i = 0; i < 10; ++i)
            {
                Ice.DispatchInterceptorAsyncCallback cb = new DispatchInterceptorAsyncCallbackI();
                   
                lastStatus_ = servant_.ice_dispatch(request, cb);
                test(lastStatus_ == Ice.DispatchStatus.DispatchAsync);
            }
            
            request.getCurrent().ctx["retry"] = "no";
        }
        

        lastStatus_ = servant_.ice_dispatch(request, this);
        return lastStatus_;
    }

    public bool response(bool ok)
    {
        setActualStatus(ok ? Ice.DispatchStatus.DispatchOK : Ice.DispatchStatus.DispatchUserException);
        return true;
    }

    public bool exception(System.Exception ex)
    {
        setActualStatus(ex);
        return true;
    }

    internal override void
    clear()
    {
        base.clear();
        lock(this)
        {
            actualStatus_ = Ice.DispatchStatus.DispatchAsync;
            exception_ = null;
        }
    }

    internal void 
    setActualStatus(Ice.DispatchStatus status)
    {
        lock(this)
        {
            actualStatus_ = status;
            exception_ = null;
        }
    }

    internal void 
    setActualStatus(System.Exception ex)
    {
        lock(this)
        {
            exception_ = ex;
            actualStatus_ = Ice.DispatchStatus.DispatchAsync;
        }
    }

    internal Ice.DispatchStatus 
    getActualStatus()
    {
        lock(this)
        {
            return actualStatus_;
        }
    }

    internal System.Exception
    getException()
    {
        lock(this)
        {
            return exception_;
        }
    }

    private Ice.DispatchStatus actualStatus_;
    private System.Exception exception_;
}
