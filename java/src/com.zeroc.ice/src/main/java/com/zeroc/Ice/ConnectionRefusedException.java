// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * The exception that is thrown when the server host actively refuses a connection.
 */
public final class ConnectionRefusedException extends ConnectFailedException {
    /**
     * Constructs a ConnectionRefusedException.
     */
    public ConnectionRefusedException() {
        super();
    }

    /**
     * Constructs a ConnectionRefusedException with a cause.
     *
     * @param cause the cause
     */
    public ConnectionRefusedException(Throwable cause) {
        super(cause);
    }

    @Override
    public String ice_id() {
        return "::Ice::ConnectionRefusedException";
    }

    private static final long serialVersionUID = 2368977428979563391L;
}
