// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.interceptor;

import test.Ice.interceptor.Test.RetryException;

//
// A dispatch interceptor with special handling for AMD requests
//

class AMDInterceptorI extends InterceptorI implements Ice.DispatchInterceptorAsyncCallback
{
    AMDInterceptorI(Ice.Object servant)
    {
        super(servant);
    }

    @Override
    public boolean
    dispatch(Ice.Request request)
        throws Ice.UserException
    {
        Ice.Current current = request.getCurrent();
        _lastOperation = current.operation;

        if(_lastOperation.equals("amdAddWithRetry"))
        {
            for(int i = 0; i < 10; ++i)
            {
                Ice.DispatchInterceptorAsyncCallback cb = new Ice.DispatchInterceptorAsyncCallback()
                {
                    @Override
                    public boolean response()
                    {
                        return false;
                    }

                    @Override
                    public boolean exception(java.lang.Exception ex)
                    {
                        test(ex instanceof RetryException);
                        return false;
                    }
                };

                try
                {
                    _lastStatus = _servant.ice_dispatch(request, cb);
                    test(_lastStatus);
                }
                catch(RetryException ex)
                {
                    //
                    // Expected, retry
                    //
                }
            }

            request.getCurrent().ctx.put("retry", "no");
        }

        _lastStatus = _servant.ice_dispatch(request, this);
        return _lastStatus;
    }

    @Override
    public boolean response()
    {
        return true;
    }

    @Override
    public boolean exception(java.lang.Exception ex)
    {
        _exception = ex;
        return true;
    }

    @Override
    void
    clear()
    {
        super.clear();
        synchronized(this)
        {
            _exception = null;
        }
    }

    synchronized java.lang.Exception
    getException()
    {
        return _exception;
    }

    private java.lang.Exception _exception;
}
