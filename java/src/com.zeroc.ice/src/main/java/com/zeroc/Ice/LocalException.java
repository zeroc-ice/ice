// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * Base class for Ice run-time exceptions. These exceptions are not checked so we inherit from
 * java.lang.RuntimeException. User exceptions are checked exceptions and therefore inherit directly
 * from java.lang.Exception.
 */
public class LocalException extends RuntimeException {
    /**
     * Constructs a LocalException with a message.
     *
     * @param message the detail message
     */
    public LocalException(String message) {
        super(message);
    }

    /**
     * Constructs a LocalException with a message and a cause.
     *
     * @param message the detail message
     * @param cause the cause
     */
    public LocalException(String message, Throwable cause) {
        super(message, cause);
    }

    /**
     * Returns the type ID of this exception.
     *
     * @return the type ID of this exception
     */
    public String ice_id() {
        return "::Ice::LocalException";
    }

    private static final long serialVersionUID = 0L;
}
