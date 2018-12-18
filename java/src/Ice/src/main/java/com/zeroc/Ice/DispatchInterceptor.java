// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.Ice;

import java.util.concurrent.CompletionStage;

/**
 * Base class that allows a server to intercept incoming requests.
 * The application must derive a concrete class from <code>DispatchInterceptor</code>
 * that implements the {@link DispatchInterceptor#dispatch} operation. An instance of this derived
 * class can be registered with an object adapter like any other servant.
 * <p>
 * A dispatch interceptor is useful particularly to automatically retry requests
 * that have failed due to a recoverable error condition.
 **/
public abstract class DispatchInterceptor implements com.zeroc.Ice.Object
{
    /**
     * Called by the Ice run time to dispatch an incoming request. The implementation
     * of <code>dispatch</code> must dispatch the request to the actual servant.
     *
     * @param request The details of the incoming request.
     * @return A completion stage if dispatched asynchronously, null otherwise.
     * @throws UserException A user exception that propagates out of this method will be
     * marshaled as the result.
     *
     * @see Request
     **/
    public abstract CompletionStage<OutputStream> dispatch(Request request)
        throws UserException;

    /** @hidden */
    @Override
    public CompletionStage<OutputStream> _iceDispatch(com.zeroc.IceInternal.Incoming in, Current current)
        throws UserException
    {
        return dispatch(in);
    }
}
