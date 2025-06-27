// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * The exception that is thrown when a dispatch cannot find the operation carried by the request on the target
 * servant. This is typically due to a mismatch in the Slice definitions, such as the client using Slice
 * definitions newer than the server's.
 */
public final class OperationNotExistException extends RequestFailedException {
    /**
     * Constructs an OperationNotExistException.
     */
    public OperationNotExistException() {
        super(ReplyStatus.OperationNotExist);
    }

    /**
     * Constructs an OperationNotExistException with object details.
     *
     * @param id        the identity of the Ice Object to which the request was sent
     * @param facet     the facet to which the request was sent
     * @param operation the operation name of the request
     */
    public OperationNotExistException(Identity id, String facet, String operation) {
        super(ReplyStatus.OperationNotExist, id, facet, operation);
    }

    @Override
    public String ice_id() {
        return "::Ice::OperationNotExistException";
    }

    private static final long serialVersionUID = 3973646568523472620L;
}
