// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** This exception is raised if no suitable endpoint is available. */
public final class NoEndpointException extends LocalException {
    /**
     * Constructs a NoEndpointException with a message.
     *
     * @param message the detail message for this exception
     */
    public NoEndpointException(String message) {
        super(message);
    }

    /**
     * Constructs a NoEndpointException with a proxy.
     *
     * @param proxy the proxy for which no suitable endpoint is available
     */
    public NoEndpointException(ObjectPrx proxy) {
        super("No suitable endpoint available for proxy '" + proxy + "'");
    }

    @Override
    public String ice_id() {
        return "::Ice::NoEndpointException";
    }

    private static final long serialVersionUID = -5026638954785808518L;
}
