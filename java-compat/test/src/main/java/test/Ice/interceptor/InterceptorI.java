//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.interceptor;

import test.Ice.interceptor.Test.RetryException;

class InterceptorI extends Ice.DispatchInterceptor
{
    InterceptorI(Ice.Object servant)
    {
        _servant = servant;
    }

    protected static void
    test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    @Override
    public boolean
    dispatch(Ice.Request request)
        throws Ice.UserException
    {
        Ice.Current current = request.getCurrent();
        _lastOperation = current.operation;

        if(_lastOperation.equals("addWithRetry"))
        {
            for(int i = 0; i < 10; ++i)
            {
                try
                {
                    _servant.ice_dispatch(request);
                    test(false);
                }
                catch(RetryException re)
                {
                    //
                    // Expected, retry
                    //
                }
            }

            current.ctx.put("retry", "no");
        }

        _lastStatus = _servant.ice_dispatch(request);
        return _lastStatus;
    }

    boolean
    getLastStatus()
    {
        return _lastStatus;
    }

    String
    getLastOperation()
    {
        return _lastOperation;
    }

    void
    clear()
    {
        _lastOperation = null;
        _lastStatus = false;
    }

    protected final Ice.Object _servant;
    protected String _lastOperation;
    protected boolean _lastStatus;
}
