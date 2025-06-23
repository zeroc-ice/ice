// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** Reports a failure that occurred while parsing a string. */
public final class ParseException extends LocalException {
    /**
     * Constructs a {@code ParseException} with a message.
     *
     * @param message the detail message for this exception
     */
    public ParseException(String message) {
        super(message);
    }

    /**
     * Constructs a {@code ParseException} with a message and a cause.
     *
     * @param message the detail message for this exception
     * @param cause   the cause of this exception
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
