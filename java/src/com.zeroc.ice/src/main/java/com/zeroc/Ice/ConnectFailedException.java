// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.net.InetSocketAddress;

/** The exception that is thrown when a connection establishment fails. */
public class ConnectFailedException extends SocketException {
    /** Constructs a ConnectFailedException. */
    public ConnectFailedException() {
        super();
    }

    /**
     * Constructs a ConnectFailedException with a peer address and a cause.
     *
     * @param peerAddress the peer address (may be null)
     * @param cause the cause
     */
    public ConnectFailedException(InetSocketAddress peerAddress, Throwable cause) {
        super(peerAddress != null ? "Failed to connect to " + peerAddress + "." : null, cause);
    }

    @Override
    public String ice_id() {
        return "::Ice::ConnectFailedException";
    }

    private static final long serialVersionUID = 3367444083227777090L;
}
