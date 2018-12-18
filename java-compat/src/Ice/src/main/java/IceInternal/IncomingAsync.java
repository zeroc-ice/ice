// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public class IncomingAsync extends IncomingBase implements Ice.AMDCallback
{
    public
    IncomingAsync(Incoming in)
    {
        super(in);
        in.setAsync(this);
    }

    @Override
    public void
    ice_exception(java.lang.Exception ex)
    {
        if(_interceptorCBs != null)
        {
            try
            {
                for(Ice.DispatchInterceptorAsyncCallback cb : _interceptorCBs)
                {
                    if(!cb.exception(ex))
                    {
                        return;
                    }
                }
            }
            catch(java.lang.RuntimeException exc)
            {
                return;
            }
        }

        checkResponseSent();
        exception(ex, true); // User thread
    }

    final public void
    kill(Incoming in)
    {
        checkResponseSent();

        // Adopt observer and cookie
        in._observer = _observer;
        _observer = null;
        in._cookie = _cookie;
        _cookie = null;
    }

    final protected void
    completed()
    {
        if(_interceptorCBs != null)
        {
            try
            {
                for(Ice.DispatchInterceptorAsyncCallback cb : _interceptorCBs)
                {
                    if(!cb.response())
                    {
                        return;
                    }
                }
            }
            catch(java.lang.RuntimeException exc)
            {
                return;
            }
        }

        checkResponseSent();
        response(true); // User thread
    }

    synchronized private void
    checkResponseSent()
    {
        if(_responseSent)
        {
            throw new Ice.ResponseSentException();
        }
        _responseSent = true;
    }

    private boolean _responseSent = false;
}
