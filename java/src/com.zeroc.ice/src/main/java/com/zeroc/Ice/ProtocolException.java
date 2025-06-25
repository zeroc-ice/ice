// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** The base class for Ice protocol exceptions. */
public class ProtocolException extends LocalException {
    /**
     * Constructs a ProtocolException with a message.
     *
     * @param message the detail message
     */
    public ProtocolException(String message) {
        super(message);
    }

    /**
     * Constructs a ProtocolException with a message and a cause.
     *
     * @param message the detail message
     * @param cause   the cause
     */
    public ProtocolException(String message, Throwable cause) {
        super(message, cause);
    }

    @Override
    public String ice_id() {
        return "::Ice::ProtocolException";
    }

    private static final long serialVersionUID = 6046324714700663302L;
}
