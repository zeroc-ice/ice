// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** Reports a failure that occurred while parsing a string. */
public class ParseException extends LocalException {
    public ParseException(String message) {
        super(message);
    }

    public ParseException(String message, Throwable cause) {
        super(message, cause);
    }

    // TODO, do we really need these `ice_id` methods on local exceptions?
    public String ice_id() {
        return "::Ice::ParseException";
    }

    private static final long serialVersionUID = 7926520612608626840L;
}
