// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** This exception is raised if no suitable endpoint is available. */
public final class NoEndpointException extends LocalException {
    public NoEndpointException(String message) {
        super(message);
    }

    public String ice_id() {
        return "::Ice::NoEndpointException";
    }

    public static NoEndpointException fromProxyString(String proxyString) {
        return new NoEndpointException(
            "No suitable endpoint available for proxy '" + proxyString + "'");
    }

    private static final long serialVersionUID = -5026638954785808518L;
}
