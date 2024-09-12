// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** This exception indicates a failure in a security subsystem. */
public final class SecurityException extends LocalException {
    public SecurityException(String message) {
        super(message);
    }

    public SecurityException(String message, Throwable cause) {
        super(message, cause);
    }

    public String ice_id() {
        return "::Ice::SecurityException";
    }

    private static final long serialVersionUID = 7929245908983964710L;
}
