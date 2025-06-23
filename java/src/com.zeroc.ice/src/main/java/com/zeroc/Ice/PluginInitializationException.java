// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** This exception indicates that a failure occurred while initializing a plug-in. */
public final class PluginInitializationException extends LocalException {
    /**
     * Constructs a PluginInitializationException with a message.
     *
     * @param message the detail message for this exception
     */
    public PluginInitializationException(String message) {
        super(message);
    }

    /**
     * Constructs a PluginInitializationException with a message and a cause.
     *
     * @param message the detail message for this exception
     * @param cause   the cause of this exception
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
