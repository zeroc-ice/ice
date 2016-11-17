// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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
     * @return The return value must be whatever is returned by {@link #ice_dispatch}.
     * @throws UserException A user exception that propagates out of this method will be
     * marshaled as the result.
     *
     * @see Request
     **/
    public abstract boolean
    dispatch(Request request)
        throws Ice.UserException;

    @Override
    public boolean
    _iceDispatch(IceInternal.Incoming in, Current current)
        throws Ice.UserException
    {
        try
        {
            return dispatch(in);
        }
        catch(ResponseSentException ex)
        {
            return false;
        }
    }
}
