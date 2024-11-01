// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** This exception indicates a connection establishment timeout condition. */
public final class ConnectTimeoutException extends TimeoutException {
    public ConnectTimeoutException() {
        super("Connect timed out.");
    }

    public String ice_id() {
        return "::Ice::ConnectTimeoutException";
    }

    private static final long serialVersionUID = -1271371420507272518L;
}
