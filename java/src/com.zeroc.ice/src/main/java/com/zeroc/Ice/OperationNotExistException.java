// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * This exception is raised if an operation for a given object does not exist on the server. This is
 * typically due to a mismatch in the Slice definitions, such as the client using Slice definitions
 * newer than the server's.
 */
public final class OperationNotExistException extends RequestFailedException {
    public OperationNotExistException() {
        super("OperationNotExistException");
    }

    public OperationNotExistException(Identity id, String facet, String operation) {
        super("OperationNotExistException", id, facet, operation);
    }

    public String ice_id() {
        return "::Ice::OperationNotExistException";
    }

    private static final long serialVersionUID = 3973646568523472620L;
}
