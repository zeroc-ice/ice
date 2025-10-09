// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.util.concurrent.CompletableFuture;
import java.util.concurrent.CompletionStage;

/**
 * Base class for dynamic dispatch servants. A server application derives a concrete servant class
 * from {@code Blobject} that implements the {@link Blobject#ice_invoke} method.
 */
public interface Blobject extends Object {
    /**
     * Dispatches an incoming request.
     *
     * @param inEncaps the encoded in-parameters for the operation
     * @param current the Current object to pass to the operation
     * @return This method returns an instance of {@code Ice_invokeResult}. If the operation completed successfully,
     *     set the {@code returnValue} member to {@code true} and the {@code outParams} member to the encoded results.
     *     If the operation throws a user exception, you can either throw it directly or set the {@code returnValue}
     *     member to {@code false} and the {@code outParams} member to the encoded user exception.
     *     If the operation throws an Ice run-time exception, you must throw it directly.
     * @throws UserException A user exception can be thrown directly and the run time will marshal it.
     */
    Object.Ice_invokeResult ice_invoke(byte[] inEncaps, Current current) throws UserException;

    @Override
    default CompletionStage<OutgoingResponse> dispatch(IncomingRequest request) throws UserException {
        byte[] inEncaps = request.inputStream.readEncapsulation(null);
        Object.Ice_invokeResult r = ice_invoke(inEncaps, request.current);
        return CompletableFuture.completedFuture(
            request.current.createOutgoingResponse(r.returnValue, r.outParams));
    }
}
