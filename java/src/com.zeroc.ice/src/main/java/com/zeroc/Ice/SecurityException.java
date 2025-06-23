// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** This exception indicates a failure in a security subsystem. */
public final class SecurityException extends LocalException {
    /**
     * Constructs a {@code SecurityException} with a message.
     *
     * @param message the detail message for this exception
     */
    public SecurityException(String message) {
        super(message);
    }

    /**
     * Constructs a {@code SecurityException} with a message and a cause.
     *
     * @param message the detail message for this exception
     * @param cause   the cause of this exception
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
