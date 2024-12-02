// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** Provides access to the connection details of a WebSocket connection */
public final class WSConnectionInfo extends ConnectionInfo {
    /** The headers from the HTTP upgrade request. */
    public final java.util.Map<java.lang.String, java.lang.String> headers;

    WSConnectionInfo(
            ConnectionInfo underlying, java.util.Map<java.lang.String, java.lang.String> headers) {
        super(underlying);
        this.headers = headers;
    }
}
