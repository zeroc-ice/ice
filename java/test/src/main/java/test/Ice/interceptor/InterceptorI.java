// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.interceptor;

import java.util.concurrent.CompletionStage;

import com.zeroc.Ice.OutputStream;

import test.Ice.interceptor.Test.RetryException;

class InterceptorI extends com.zeroc.Ice.DispatchInterceptor
{
    InterceptorI(com.zeroc.Ice.Object servant)
    {
        _servant = servant;
    }

    protected static void test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    @Override
    public CompletionStage<OutputStream> dispatch(com.zeroc.Ice.Request request)
        throws com.zeroc.Ice.UserException
    {
        com.zeroc.Ice.Current current = request.getCurrent();
        _lastOperation = current.operation;

        if(_lastOperation.equals("addWithRetry") || _lastOperation.equals("amdAddWithRetry"))
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

        CompletionStage<OutputStream> f = _servant.ice_dispatch(request);
        _lastStatus = f != null;
        return f;
    }

    boolean getLastStatus()
    {
        return _lastStatus;
    }

    String getLastOperation()
    {
        return _lastOperation;
    }

    void clear()
    {
        _lastOperation = null;
        _lastStatus = false;
    }

    protected final com.zeroc.Ice.Object _servant;
    protected String _lastOperation;
    protected boolean _lastStatus;
}
