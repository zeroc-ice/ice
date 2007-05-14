// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

public abstract class DispatchInterceptor extends ObjectImpl
{
    public abstract IceInternal.DispatchStatus
    dispatch(Request request);
   
    public IceInternal.DispatchStatus
    __dispatch(IceInternal.Incoming in, Current current)
    {
        try
        {
            IceInternal.DispatchStatus status = dispatch(in);
            if(status != IceInternal.DispatchStatus.DispatchAsync)
            {
                //
                // Make sure 'in' owns the connection etc.
                //
                in.killAsync();
            }
            return status;
        }
        catch(ResponseSentException e)
        {
            return IceInternal.DispatchStatus.DispatchAsync;
        }
        catch(java.lang.RuntimeException e)
        {
            try
            {
                in.killAsync();
                throw e;
            }
            catch(ResponseSentException rse)
            {
                return IceInternal.DispatchStatus.DispatchAsync;
            }
        }
    }

    public IceInternal.DispatchStatus
    __collocDispatch(IceInternal.Direct request) 
    {
        //
        // Should only return DispatchOK or DispatchUserException
        //
        return dispatch(request);
    }


    public IceInternal.DispatchStatus 
    ice_dispatch(Ice.Request request, DispatchInterceptorAsyncCallback cb)
    {
        //
        // A dispatch interceptor can call another interceptor (as if it was a regular servant)
        // to create a chain of interceptors
        //

        if(request.isCollocated())
        {
            return __collocDispatch((IceInternal.Direct)request);
        }
        else
        {
            IceInternal.Incoming in = (IceInternal.Incoming)request;
            if(cb != null)
            {
                in.push(cb);
            }
            try
            {
                return __dispatch(in, request.getCurrent());
            }
            finally
            {
                if(cb != null)
                {
                    in.pop();
                }
            }
        }
    }
}