// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** The exception that is thrown when a timeout occurs. This is the base class for all timeout exceptions. */
public class TimeoutException extends LocalException {
    /** Constructs a TimeoutException. */
    public TimeoutException() {
        super("Operation timed out.");
    }

    /**
     * Constructs a TimeoutException with a message.
     *
     * @param message the detail message
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
