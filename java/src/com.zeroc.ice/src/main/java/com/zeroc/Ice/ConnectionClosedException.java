// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** This exception indicates that a connection was closed gracefully. */
public final class ConnectionClosedException extends LocalException {
    public ConnectionClosedException(String message, boolean closedByApplication) {
        super(message);
        this.closedByApplication = closedByApplication;
    }

    public String ice_id() {
        return "::Ice::ConnectionClosedException";
    }

    public final boolean closedByApplication;

    private static final long serialVersionUID = 7544357079121284684L;
}
