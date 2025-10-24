// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** The exception that is thrown when a connection establishment fails. */
public class ConnectFailedException extends SocketException {
    /** Constructs a ConnectFailedException. */
    public ConnectFailedException() {
        super();
    }

    /**
     * Constructs a ConnectFailedException with a cause.
     *
     * @param cause the cause
     */
    public ConnectFailedException(Throwable cause) {
        super(cause);
    }

    @Override
    public String ice_id() {
        return "::Ice::ConnectFailedException";
    }

    private static final long serialVersionUID = 3367444083227777090L;
}
