// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** This exception indicates that a failure occurred while initializing a plug-in. */
public final class PluginInitializationException extends LocalException {
    public PluginInitializationException(String message) {
        super(message);
    }

    public PluginInitializationException(String message, Throwable cause) {
        super(message, cause);
    }

    public String ice_id() {
        return "::Ice::PluginInitializationException";
    }

    private static final long serialVersionUID = 1589933368626096169L;
}
