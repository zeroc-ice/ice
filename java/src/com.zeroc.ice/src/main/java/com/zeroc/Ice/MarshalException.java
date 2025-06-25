// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** The exception that is thrown when an error occurs during marshaling or unmarshaling. */
public final class MarshalException extends ProtocolException {
    /**
     * Constructs a MarshalException with a message.
     *
     * @param message the detail message
     */
    public MarshalException(String message) {
        super(message);
    }

    /**
     * Constructs a MarshalException with a message and a cause.
     *
     * @param message the detail message
     * @param cause the cause
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
