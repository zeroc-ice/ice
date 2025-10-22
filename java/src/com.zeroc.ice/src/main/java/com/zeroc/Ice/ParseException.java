// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** Reports a failure that occurred while parsing a string. */
public final class ParseException extends LocalException {
    /**
     * Constructs a ParseException with a message.
     *
     * @param message the detail message
     */
    public ParseException(String message) {
        super(message);
    }

    /**
     * Constructs a ParseException with a message and a cause.
     *
     * @param message the detail message
     * @param cause the cause
     */
    public ParseException(String message, Throwable cause) {
        super(message, cause);
    }

    @Override
    public String ice_id() {
        return "::Ice::ParseException";
    }

    private static final long serialVersionUID = 7926520612608626840L;
}
