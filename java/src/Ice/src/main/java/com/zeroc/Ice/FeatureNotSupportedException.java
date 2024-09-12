// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** This exception is raised if an unsupported feature is used. */
public final class FeatureNotSupportedException extends LocalException {
    public FeatureNotSupportedException(String message) {
        super(message);
    }

    public String ice_id() {
        return "::Ice::FeatureNotSupportedException";
    }

    private static final long serialVersionUID = -4629958372080397318L;
}
