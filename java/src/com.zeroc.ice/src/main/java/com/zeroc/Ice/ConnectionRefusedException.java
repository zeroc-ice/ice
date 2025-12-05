// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.net.InetSocketAddress;

/** The exception that is thrown when the server host actively refuses a connection. */
public final class ConnectionRefusedException extends ConnectFailedException {
    /**
     * Constructs a ConnectionRefusedException.
     */
    public ConnectionRefusedException() {
        super();
    }

    /**
     * Constructs a ConnectionRefusedException with a remote server address and a cause.
     *
     * @param serverAddress the remote server address (may be null)
     * @param cause the cause
     */
    public ConnectionRefusedException(InetSocketAddress serverAddress, Throwable cause) {
        super(serverAddress, cause);
    }

    @Override
    public String ice_id() {
        return "::Ice::ConnectionRefusedException";
    }

    private static final long serialVersionUID = 2368977428979563391L;
}
