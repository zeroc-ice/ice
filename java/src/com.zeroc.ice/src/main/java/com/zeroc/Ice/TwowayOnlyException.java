// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * The exception that is thrown when attempting to invoke an operation with {@code ice_oneway}, {@code ice_batchOneway},
 * {@code ice_datagram}, or {@code ice_batchDatagram}, and the operation has a return value, an out parameter, or an exception
 * specification.
 */
public final class TwowayOnlyException extends LocalException {
    /**
     * Constructs a TwowayOnlyException with the operation name.
     *
     * @param operation the operation that can only be invoked with a twoway request
     */
    public TwowayOnlyException(String operation) {
        super(
            "Cannot invoke operation '"
                + operation
                + "' with a oneway, batchOneway, datagram, of batchDatagram proxy.");
        this.operation = operation;
    }

    @Override
    public String ice_id() {
        return "::Ice::TwowayOnlyException";
    }

    /** The name of the operation that was invoked. */
    public final String operation;

    private static final long serialVersionUID = -7036652448391478186L;
}
