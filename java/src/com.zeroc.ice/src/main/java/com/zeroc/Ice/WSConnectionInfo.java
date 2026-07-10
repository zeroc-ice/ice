// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.util.Map;

/** Provides access to the connection details of a WebSocket connection. */
public final class WSConnectionInfo extends ConnectionInfo {
    /**
     * The HTTP headers from the WebSocket upgrade handshake: the request headers for an incoming connection, and the
     * response headers for an outgoing connection.
     */
    public final Map<java.lang.String, java.lang.String> headers;

    WSConnectionInfo(ConnectionInfo underlying, Map<java.lang.String, java.lang.String> headers) {
        super(underlying);
        this.headers = headers;
    }
}
