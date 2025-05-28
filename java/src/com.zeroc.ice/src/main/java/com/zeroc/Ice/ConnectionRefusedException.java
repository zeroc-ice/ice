// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * This exception indicates a connection failure for which the server host actively refuses a
 * connection.
 */
public final class ConnectionRefusedException extends ConnectFailedException {
    /**
     * Constructs a new {@code ConnectionRefusedException}.
     */
    public ConnectionRefusedException() {
        super();
    }

    /**
     * Constructs a new {@code ConnectionRefusedException} with a cause.
     *
     * @param cause the cause of the exception
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
