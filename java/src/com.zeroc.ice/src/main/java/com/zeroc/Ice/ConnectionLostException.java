// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** The exception that is thrown when an established connection is lost. */
public final class ConnectionLostException extends SocketException {
    /**
     * Constructs a ConnectionLostException.
     */
    public ConnectionLostException() {
        super();
    }

    /**
     * Constructs a ConnectionLostException with a cause.
     *
     * @param cause the cause
     */
    public ConnectionLostException(Throwable cause) {
        super(cause);
    }

    @Override
    public String ice_id() {
        return "::Ice::ConnectionLostException";
    }

    private static final long serialVersionUID = -7871526265585148499L;
}
