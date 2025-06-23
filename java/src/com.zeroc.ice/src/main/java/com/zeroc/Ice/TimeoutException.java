// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** This exception indicates a timeout condition. */
public class TimeoutException extends LocalException {
    /**
     * Constructs a TimeoutException.
     */
    public TimeoutException() {
        super("Operation timed out.");
    }

    /**
     * Constructs a TimeoutException with a message.
     *
     * @param message the detail message for this exception
     */
    public TimeoutException(String message) {
        super(message);
    }

    @Override
    public String ice_id() {
        return "::Ice::TimeoutException";
    }

    private static final long serialVersionUID = 1390295317871659332L;
}
