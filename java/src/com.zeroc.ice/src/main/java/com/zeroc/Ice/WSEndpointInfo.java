// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** Provides access to a WebSocket endpoint information. */
public abstract class WSEndpointInfo extends EndpointInfo {
    /** The URI configured with the endpoint. */
    public String resource = "";
}
