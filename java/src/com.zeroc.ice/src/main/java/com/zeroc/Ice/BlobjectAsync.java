// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.util.concurrent.CompletionStage;

/**
 * <code>BlobjectAsync</code> is the base class for asynchronous dynamic dispatch servants. A server
 * application derives a concrete servant class that implements the {@link
 * BlobjectAsync#ice_invokeAsync} method, which is called by the Ice run time to deliver every
 * request on this object.
 */
public interface BlobjectAsync extends Object {
    /**
     * Dispatch an incoming request.
     *
     * @param inEncaps The encoded input parameters.
     * @param current The Current object, which provides important information about the request,
     *     such as the identity of the target object and the name of the operation.
     * @return A completion stage that eventually completes with the result of the invocation, an
     *     instance of <code>Ice_invokeResult</code>. If the operation completed successfully, set
     *     the <code>returnValue</code> member to <code>true</code> and the <code>outParams</code>
     *     member to the encoded results. If the operation raises a user exception, you can throw it
     *     directly from <code>ice_invokeAsync</code>, or complete the future by setting the <code>
     *     returnValue
     *     </code> member to <code>false</code> and the <code>outParams</code> member to the encoded
     *     user exception.
     * @throws UserException A user exception raised by this method will be marshaled as the result
     *     of the invocation.
     */
    CompletionStage<Object.Ice_invokeResult> ice_invokeAsync(byte[] inEncaps, Current current)
        throws UserException;

    @Override
    default CompletionStage<OutgoingResponse> dispatch(IncomingRequest request)
        throws UserException {
        byte[] inEncaps = request.inputStream.readEncapsulation(null);
        return ice_invokeAsync(inEncaps, request.current)
            .thenApply(r -> request.current.createOutgoingResponse(r.returnValue, r.outParams));
    }
}
