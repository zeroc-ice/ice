// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** The exception that is thrown when attempting to use an unsupported feature. */
public final class FeatureNotSupportedException extends LocalException {
    /**
     * Constructs a FeatureNotSupportedException with a message.
     *
     * @param message the detail message
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
