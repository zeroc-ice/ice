// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.util.concurrent.CompletableFuture;
import java.util.concurrent.CompletionStage;

/**
 * Base class for dynamic dispatch servants.
 *
 * <p>This class is provided for backward compatibility.
 * You should consider deriving directly from {@link Object} and overriding the {@link Object#dispatch} method.
 */
public interface Blobject extends Object {
    /**
     * Dispatches an incoming request.
     *
     * @param inEncaps an encapsulation containing the encoded in-parameters for the operation.
     * @param current the Current object of the incoming request
     * @return The method returns an instance of {@link Ice_invokeResult}. If the operation completed successfully,
     *     set its {@code returnValue} field to {@code true} and its {@code outParams} field to the encoded results.
     *     If the operation threw a user exception, you can either throw it directly or set the {@code returnValue}
     *     field to {@code false} and the {@code outParams} field to the encoded user exception.
     * @throws UserException If a user exception is thrown, Ice will marshal it as the response payload.
     */
    Object.Ice_invokeResult ice_invoke(byte[] inEncaps, Current current) throws UserException;

    @Override
    default CompletionStage<OutgoingResponse> dispatch(IncomingRequest request) throws UserException {
        byte[] inEncaps = request.inputStream.readEncapsulation(null);
        Object.Ice_invokeResult r = ice_invoke(inEncaps, request.current);
        return CompletableFuture.completedFuture(request.current.createOutgoingResponse(r.returnValue, r.outParams));
    }
}
