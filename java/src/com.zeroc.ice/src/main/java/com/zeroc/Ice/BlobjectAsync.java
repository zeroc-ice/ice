// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.util.concurrent.CompletionStage;

/**
 * Base class for asynchronous dynamic dispatch servants.
 * A server application derives a concrete servant class that implements the {@link BlobjectAsync#ice_invokeAsync}
 * method, which is called by the Ice run time to deliver every request on this object.
 */
public interface BlobjectAsync extends Object {
    /**
     * Dispatches an incoming request.
     *
     * @param inEncaps the encoded in-parameters for the operation
     * @param current the Current object to pass to the operation
     * @return a completion stage completes with the result of the invocation, an instance of {@code Ice_invokeResult}.
     *     If the operation completed successfully, set the {@code returnValue} member to {@code true} and the
     *     {@code outParams} member to the encoded results.
     *     If the operation throws a user exception, you can throw it directly from {@code ice_invokeAsync},
     *     or complete the future by setting the {@code returnValue} member to {@code false} and the {@code outParams}
     *     member to the encoded user exception.
     * @throws UserException A user exception can be thrown directly and the run time will marshal it.
     */
    CompletionStage<Object.Ice_invokeResult> ice_invokeAsync(byte[] inEncaps, Current current) throws UserException;

    @Override
    default CompletionStage<OutgoingResponse> dispatch(IncomingRequest request) throws UserException {
        byte[] inEncaps = request.inputStream.readEncapsulation(null);
        return ice_invokeAsync(inEncaps, request.current)
            .thenApply(r -> request.current.createOutgoingResponse(r.returnValue, r.outParams));
    }
}
