// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.Ice;

import java.util.concurrent.CompletionStage;
import java.util.concurrent.CompletableFuture;

/**
 * <code>BlobjectAsync</code> is the base class for asynchronous dynamic
 * dispatch servants. A server application derives a concrete servant
 * class that implements the {@link BlobjectAsync#ice_invokeAsync} method,
 * which is called by the Ice run time to deliver every request on this
 * object.
 **/
public interface BlobjectAsync extends com.zeroc.Ice.Object
{
    /**
     * Dispatch an incoming request.
     *
     * @param inEncaps The encoded input parameters.
     * @param current The Current object, which provides important information
     * about the request, such as the identity of the target object and the
     * name of the operation.
     * @return A completion stage that eventually completes with the result of
     * the invocation, an instance of <code>Ice_invokeResult</code>.
     * If the operation completed successfully, set the <code>returnValue</code>
     * member to <code>true</code> and the <code>outParams</code> member to
     * the encoded results. If the operation raises a user exception, you can
     * throw it directly from <code>ice_invokeAsync</code>, or complete the
     * future by setting the <code>returnValue</code> member to
     * <code>false</code> and the <code>outParams</code> member to the encoded
     * user exception.
     * @throws UserException A user exception raised by this method will be marshaled
     * as the result of the invocation.
     **/
    CompletionStage<Object.Ice_invokeResult> ice_invokeAsync(byte[] inEncaps, Current current)
        throws UserException;

    /** @hidden */
    @Override
    default CompletionStage<OutputStream> _iceDispatch(com.zeroc.IceInternal.Incoming in, Current current)
        throws UserException
    {
        byte[] inEncaps = in.readParamEncaps();
        CompletableFuture<OutputStream> f = new CompletableFuture<>();
        ice_invokeAsync(inEncaps, current).whenComplete((result, ex) ->
            {
                if(ex != null)
                {
                    f.completeExceptionally(ex);
                }
                else
                {
                    f.complete(in.writeParamEncaps(result.outParams, result.returnValue));
                }
            });
        return f;
    }
}
