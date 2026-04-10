// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * The exception that is thrown when attempting to invoke a oneway-only operation (an operation with the
 * {@code ["oneway"]} metadata directive) using a twoway proxy.
 */
public final class OnewayOnlyException extends LocalException {
    /**
     * Constructs an OnewayOnlyException with the operation name.
     *
     * @param operation the name of the oneway-only operation
     */
    public OnewayOnlyException(String operation) {
        super("Cannot invoke oneway operation '" + operation + "' with a twoway proxy.");
        this.operation = operation;
    }

    @Override
    public String ice_id() {
        return "::Ice::OnewayOnlyException";
    }

    /** The name of the oneway-only operation. */
    public final String operation;

    private static final long serialVersionUID = 4519715553853048503L;
}
