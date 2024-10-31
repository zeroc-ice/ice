// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** This exception indicates a timeout condition. */
public class TimeoutException extends LocalException {
    public TimeoutException() {
        super("Operation timed out.");
    }

    public TimeoutException(String message) {
        super(message);
    }

    public String ice_id() {
        return "::Ice::TimeoutException";
    }

    private static final long serialVersionUID = 1390295317871659332L;
}
