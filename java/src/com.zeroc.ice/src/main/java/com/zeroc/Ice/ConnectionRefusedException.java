// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * This exception indicates a connection failure for which the server host actively refuses a
 * connection.
 */
public final class ConnectionRefusedException extends ConnectFailedException {
    public ConnectionRefusedException() {
        super();
    }

    public ConnectionRefusedException(Throwable cause) {
        super(cause);
    }

    public String ice_id() {
        return "::Ice::ConnectionRefusedException";
    }

    private static final long serialVersionUID = 2368977428979563391L;
}
