// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * The exception that is thrown when the connection has been gracefully shut down by the server. The request
 * that returned this exception has not been executed by the server. In most cases you will not get this exception,
 * because the client will automatically retry the invocation. However, if upon retry the server shuts down the
 * connection again, and the retry limit has been reached, then this exception is propagated to the application
 * code.
 */
public final class CloseConnectionException extends ProtocolException {
    /**
     * Constructs a CloseConnectionException.
     */
    public CloseConnectionException() {
        super("Connection closed by the peer.");
    }

    @Override
    public String ice_id() {
        return "::Ice::CloseConnectionException";
    }

    private static final long serialVersionUID = 4166975853591251903L;
}
