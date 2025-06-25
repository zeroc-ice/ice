// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** The exception that is thrown when a graceful connection closure times out. */
public final class CloseTimeoutException extends TimeoutException {
    /**
     * Constructs a CloseTimeoutException.
     */
    public CloseTimeoutException() {
        super("Close timed out.");
    }

    @Override
    public String ice_id() {
        return "::Ice::CloseTimeoutException";
    }

    private static final long serialVersionUID = -702193953324375086L;
}
