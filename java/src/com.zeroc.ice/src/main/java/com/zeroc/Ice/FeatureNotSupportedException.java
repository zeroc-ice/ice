// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** This exception is raised if an unsupported feature is used. */
public final class FeatureNotSupportedException extends LocalException {
    /**
     * Constructs a new {@code FeatureNotSupportedException} with a message.
     *
     * @param message the message explaining why the feature is not supported
     */
    public FeatureNotSupportedException(String message) {
        super(message);
    }

    @Override
    public String ice_id() {
        return "::Ice::FeatureNotSupportedException";
    }

    private static final long serialVersionUID = -4629958372080397318L;
}
