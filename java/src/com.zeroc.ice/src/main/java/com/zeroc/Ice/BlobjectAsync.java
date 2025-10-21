// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.util.concurrent.CompletionStage;

/**
 * Base class for asynchronous dynamic dispatch servants.
 *
 * <p>This class is provided for backward compatibility.
 * You should consider deriving directly from {@link Object} and overriding the {@link Object#dispatch} method.
 */
public interface BlobjectAsync extends Object {
    /**
     * Dispatches an incoming request.
     *
     * @param inEncaps an encapsulation containing the encoded in-parameters for the operation.
     * @param current the Current object of the incoming request
     * @return A CompletionStage that eventually completes with an instance of {@link Ice_invokeResult}.
     *     If the operation completed successfully, set the {@code returnValue} member to {@code true} and the
     *     {@code outParams} member to the encoded results. If the operation threw a user exception, you can either
     *     throw it directly or set the {@code returnValue} member to {@code false} and the {@code outParams} member to
     *     the encoded user exception. If the operation throws an Ice run-time exception, it must throw it directly.
     * @throws UserException If a user exception is thrown, Ice will marshal it as the response payload.
     */
    CompletionStage<Object.Ice_invokeResult> ice_invokeAsync(byte[] inEncaps, Current current) throws UserException;

    @Override
    default CompletionStage<OutgoingResponse> dispatch(IncomingRequest request) throws UserException {
        byte[] inEncaps = request.inputStream.readEncapsulation(null);
        return ice_invokeAsync(inEncaps, request.current)
            .thenApply(r -> request.current.createOutgoingResponse(r.returnValue, r.outParams));
    }
}
