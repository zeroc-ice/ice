// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** The exception that is thrown when an operation fails because the connection has been closed gracefully. */
public final class ConnectionClosedException extends LocalException {
    /**
     * Constructs a ConnectionClosedException.
     *
     * @param message the detail message
     * @param closedByApplication indicates whether the connection was closed by the application
     */
    public ConnectionClosedException(String message, boolean closedByApplication) {
        super(message);
        this.closedByApplication = closedByApplication;
    }

    @Override
    public String ice_id() {
        return "::Ice::ConnectionClosedException";
    }

    /**
     * Indicates whether the connection was closed by the application.
     */
    public final boolean closedByApplication;

    private static final long serialVersionUID = 7544357079121284684L;
}
