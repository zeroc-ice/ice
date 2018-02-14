// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
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
    public Ice.DispatchStatus
    dispatch(Ice.Request request)
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
                        public boolean response(boolean ok)
                        {
                            test(ok);
                            return false;
                        }
                        
                        @Override
                        public boolean exception(java.lang.Exception ex)
                        {
                            test(ex instanceof RetryException);
                            return false;
                        }
                    };
                
                _lastStatus = _servant.ice_dispatch(request, cb);
                test(_lastStatus == Ice.DispatchStatus.DispatchAsync);
            }
            
            request.getCurrent().ctx.put("retry", "no");
        }
        

        _lastStatus = _servant.ice_dispatch(request, this);
        return _lastStatus;
    }

    @Override
    public boolean response(boolean ok)
    {
        setActualStatus(ok ? Ice.DispatchStatus.DispatchOK : Ice.DispatchStatus.DispatchUserException);
        return true;
    }

    @Override
    public boolean exception(java.lang.Exception ex)
    {
        setActualStatus(ex);
        return true;
    }

    @Override
    void
    clear()
    {
        super.clear();
        synchronized(this)
        {
            _actualStatus = null;
            _exception = null;
        }
    }


    synchronized void 
    setActualStatus(Ice.DispatchStatus status)
    {
        _actualStatus = status;
        _exception = null;
    }

    synchronized void 
    setActualStatus(java.lang.Exception ex)
    {
        _exception = ex;
        _actualStatus = null;
    }

    synchronized Ice.DispatchStatus 
    getActualStatus()
    {
        return _actualStatus;
    }

    synchronized java.lang.Exception
    getException()
    {
        return _exception;
    }

    private Ice.DispatchStatus _actualStatus;
    private java.lang.Exception _exception;
}
