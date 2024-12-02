// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** Provides access to a WebSocket endpoint information. */
public final class WSEndpointInfo extends EndpointInfo {
    /** The URI configured with the endpoint. */
    public final String resource;

    // internal constructor
    WSEndpointInfo(EndpointInfo underlying, String resource) {
        super(underlying);
        this.resource = resource;
    }
}
