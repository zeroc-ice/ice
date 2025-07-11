// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** The exception that is thrown when a connection establishment times out. */
public final class ConnectTimeoutException extends TimeoutException {
    /**
     * Constructs a ConnectTimeoutException.
     */
    public ConnectTimeoutException() {
        super("Connect timed out.");
    }

    @Override
    public String ice_id() {
        return "::Ice::ConnectTimeoutException";
    }

    private static final long serialVersionUID = -1271371420507272518L;
}
