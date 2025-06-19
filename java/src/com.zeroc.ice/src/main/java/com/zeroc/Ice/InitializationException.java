// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** This exception is raised when a failure occurs during initialization. */
public final class InitializationException extends LocalException {
    /**
     * Constructs an {@code InitializationException} with a message.
     *
     * @param message the detail message for this exception
     */
    public InitializationException(String message) {
        super(message);
    }

    /**
     * Constructs an {@code InitializationException} with a message and a cause.
     *
     * @param message the detail message for this exception
     * @param cause   the cause of this exception
     */
    public InitializationException(String message, Throwable cause) {
        super(message, cause);
    }

    @Override
    public String ice_id() {
        return "::Ice::InitializationException";
    }

    private static final long serialVersionUID = 578611869232039264L;
}
