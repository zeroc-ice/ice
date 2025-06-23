// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** This exception is raised for errors during marshaling or unmarshaling. */
public final class MarshalException extends ProtocolException {
    /**
     * Constructs a {@code MarshalException} with a message.
     *
     * @param message the detail message for this exception
     */
    public MarshalException(String message) {
        super(message);
    }

    /**
     * Constructs a {@code MarshalException} with a message and a cause.
     *
     * @param message the detail message for this exception
     * @param cause   the cause of this exception
     */
    public MarshalException(String message, Throwable cause) {
        super(message, cause);
    }

    @Override
    public String ice_id() {
        return "::Ice::MarshalException";
    }

    private static final long serialVersionUID = -1332260000897066889L;
}
