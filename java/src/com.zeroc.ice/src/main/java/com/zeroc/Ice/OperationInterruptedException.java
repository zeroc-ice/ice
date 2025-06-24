// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** This exception indicates a request was interrupted. */
public final class OperationInterruptedException extends LocalException {
    /**
     * Constructs an OperationInterruptedException with a cause.
     *
     * @param cause the cause
     */
    public OperationInterruptedException(Throwable cause) {
        super(null, cause);
    }

    @Override
    public String ice_id() {
        return "::Ice::OperationInterruptedException";
    }

    private static final long serialVersionUID = -1099536335133286580L;
}
