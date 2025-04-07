// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.util.concurrent.CompletableFuture;
import java.util.concurrent.CompletionStage;

/**
 * Base class for dynamic dispatch servants. A server application derives a concrete servant class
 * from <code>Blobject</code> that implements the {@link Blobject#ice_invoke} method.
 */
public interface Blobject extends Object {
    /**
     * Dispatch an incoming request.
     *
     * @param inEncaps The encoded in-parameters for the operation.
     * @param current The Current object to pass to the operation.
     * @return The method returns an instance of <code>Ice_invokeResult</code>. If the operation
     *     completed successfully, set the <code>returnValue</code> member to <code>true</code> and
     *     the <code>outParams</code> member to the encoded results. If the operation raises a user
     *     exception, you can either throw it directly or set the <code>returnValue</code> member to
     *     <code>false</code> and the <code>outParams</code> member to the encoded user exception.
     *     If the operation raises an Ice run-time exception, it must throw it directly.
     * @throws UserException A user exception can be raised directly and the run time will marshal
     *     it.
     */
    Object.Ice_invokeResult ice_invoke(byte[] inEncaps, Current current)
            throws UserException;

    @Override
    default CompletionStage<OutgoingResponse> dispatch(IncomingRequest request)
            throws UserException {
        byte[] inEncaps = request.inputStream.readEncapsulation(null);
        Object.Ice_invokeResult r = ice_invoke(inEncaps, request.current);
        return CompletableFuture.completedFuture(
                request.current.createOutgoingResponse(r.returnValue, r.outParams));
    }
}
