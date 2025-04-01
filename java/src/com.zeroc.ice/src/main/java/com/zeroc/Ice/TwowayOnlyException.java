// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * The operation can only be invoked with a twoway request. This exception is raised if an attempt
 * is made to invoke an operation with <code>ice_oneway</code>, <code>ice_batchOneway</code>, <code>
 * ice_datagram</code>, or <code>ice_batchDatagram</code> and the operation has a return value,
 * out-parameters, or an exception specification.
 */
public final class TwowayOnlyException extends LocalException {
    public TwowayOnlyException(String operation) {
        super(
                "Cannot invoke operation '" +
                        operation +
                        "' with a oneway, batchOneway, datagram, of batchDatagram proxy.");
        this.operation = operation;
    }

    public String ice_id() {
        return "::Ice::TwowayOnlyException";
    }

    /** The name of the operation that was invoked. */
    public final String operation;

    private static final long serialVersionUID = -7036652448391478186L;
}
