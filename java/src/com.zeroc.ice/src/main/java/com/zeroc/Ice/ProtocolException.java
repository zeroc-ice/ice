// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** The base class for Ice protocol exceptions. */
public class ProtocolException extends LocalException {
    /**
     * Constructs a {@code ProtocolException} with a message.
     *
     * @param message the detail message for this exception
     */
    public ProtocolException(String message) {
        super(message);
    }

    /**
     * Constructs a {@code ProtocolException} with a message and a cause.
     *
     * @param message the detail message for this exception
     * @param cause   the cause of this exception
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
