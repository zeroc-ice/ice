// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** The exception that is thrown when a failure occurs in the security subsystem. This includes IceSSL errors. */
public final class SecurityException extends LocalException {
    /**
     * Constructs a SecurityException with a message.
     *
     * @param message the detail message
     */
    public SecurityException(String message) {
        super(message);
    }

    /**
     * Constructs a SecurityException with a message and a cause.
     *
     * @param message the detail message
     * @param cause   the cause
     */
    public SecurityException(String message, Throwable cause) {
        super(message, cause);
    }

    @Override
    public String ice_id() {
        return "::Ice::SecurityException";
    }

    private static final long serialVersionUID = 7929245908983964710L;
}
