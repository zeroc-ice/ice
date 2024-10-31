// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** This exception indicates that a connection was closed forcefully. */
public final class ConnectionAbortedException extends LocalException {
    public ConnectionAbortedException(String message, boolean closedByApplication) {
        super(message);
        this.closedByApplication = closedByApplication;
    }

    public String ice_id() {
        return "::Ice::ConnectionAbortedException";
    }

    public final boolean closedByApplication;

    private static final long serialVersionUID = 7912128468475443570L;
}
