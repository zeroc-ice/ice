//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.interceptor;

import test.Ice.interceptor.Test.RetryException;
import test.Ice.interceptor.Test.InvalidInputException;

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

        String context = current.ctx.get("raiseBeforeDispatch");
        if(context != null)
        {
            if(context.equals("user"))
            {
                throw new InvalidInputException();
            }
            else if(context.equals("notExist"))
            {
                throw new Ice.ObjectNotExistException();
            }
            else if(context.equals("system"))
            {
                throw new MySystemException();
            }
        }

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

                _lastStatus = _servant.ice_dispatch(request, cb);
                test(!_lastStatus);
            }

            request.getCurrent().ctx.put("retry", "no");
        }

        _lastStatus = _servant.ice_dispatch(request, this);

        context = current.ctx.get("raiseAfterDispatch");
        if(context != null)
        {
            if(context.equals("user"))
            {
                throw new InvalidInputException();
            }
            else if(context.equals("notExist"))
            {
                throw new Ice.ObjectNotExistException();
            }
            else if(context.equals("system"))
            {
                throw new MySystemException();
            }
        }

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
