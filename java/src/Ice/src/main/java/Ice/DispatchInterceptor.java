// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

/**
 * Base class that allows a server intercept incoming requests.
 * The application must derive a concrete class from <code>DispatchInterceptor</code>
 * that implements the {@link DispatchInterceptor#dispatch} operation. An instance of this derived
 * class can be registered with an object adapter like any other servant.
 * <p>
 * A dispatch interceptor is useful particularly to automatically retry requests
 * that have failed due to a recoverable error condition.
 **/
public abstract class DispatchInterceptor extends ObjectImpl
{
    /**
     * Called by the Ice run time to dispatch an incoming request. The implementation
     * of <code>dispatch</code> must dispatch the request to the actual servant.
     *
     * @param request The details of the incoming request.
     * @return For synchronous dispatch, the return value must be whatever is
     * returned {@link #ice_dispatch}. For asynchronous dispatch, the return
     * value must be <code>DispatchAsync</code>.
     *
     * @see Request
     * @see DispatchStatus
     **/
    public abstract DispatchStatus
    dispatch(Request request);

    @Override
    public DispatchStatus
    __dispatch(IceInternal.Incoming in, Current current)
    {
        try
        {
            DispatchStatus status = dispatch(in);
            if(status != DispatchStatus.DispatchAsync)
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
            return DispatchStatus.DispatchAsync;
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
                return DispatchStatus.DispatchAsync;
            }
        }
    }
}
