// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** The exception that is thrown when the initialization of an Ice plugin fails. */
public final class PluginInitializationException extends LocalException {
    /**
     * Constructs a PluginInitializationException with a message.
     *
     * @param message the detail message
     */
    public PluginInitializationException(String message) {
        super(message);
    }

    /**
     * Constructs a PluginInitializationException with a message and a cause.
     *
     * @param message the detail message
     * @param cause the cause
     */
    public PluginInitializationException(String message, Throwable cause) {
        super(message, cause);
    }

    @Override
    public String ice_id() {
        return "::Ice::PluginInitializationException";
    }

    private static final long serialVersionUID = 1589933368626096169L;
}
