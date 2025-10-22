// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** The exception that is thrown when an operation fails because the connection has been aborted. */
public final class ConnectionAbortedException extends LocalException {
    /**
     * Constructs a ConnectionAbortedException.
     *
     * @param message the detail message
     * @param closedByApplication {@code true} if the connection was aborted by the application,
     *     {@code false} if the connection was aborted by the Ice runtime.
     */
    public ConnectionAbortedException(String message, boolean closedByApplication) {
        super(message);
        this.closedByApplication = closedByApplication;
    }

    @Override
    public String ice_id() {
        return "::Ice::ConnectionAbortedException";
    }

    /** Indicates whether the connection was closed by the application. */
    public final boolean closedByApplication;

    private static final long serialVersionUID = 7912128468475443570L;
}
