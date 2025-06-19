// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** This exception is raised if no suitable endpoint is available. */
public final class NoEndpointException extends LocalException {
    /**
     * Constructs a {@code NoEndpointException} with a message.
     *
     * @param message the detail message for this exception
     */
    public NoEndpointException(String message) {
        super(message);
    }

    @Override
    public String ice_id() {
        return "::Ice::NoEndpointException";
    }

    /**
     * Creates a {@code NoEndpointException} from an invalid proxy string.
     *
     * @param proxyString the invalid proxy string
     */
    public static NoEndpointException fromProxyString(String proxyString) {
        return new NoEndpointException(
            "No suitable endpoint available for proxy '" + proxyString + "'");
    }

    private static final long serialVersionUID = -5026638954785808518L;
}
