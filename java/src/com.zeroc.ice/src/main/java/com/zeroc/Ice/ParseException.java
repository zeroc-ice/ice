// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** Reports a failure that occurred while parsing a string. */
public final class ParseException extends LocalException {
    public ParseException(String message) {
        super(message);
    }

    public ParseException(String message, Throwable cause) {
        super(message, cause);
    }

    public String ice_id() {
        return "::Ice::ParseException";
    }

    private static final long serialVersionUID = 7926520612608626840L;
}
