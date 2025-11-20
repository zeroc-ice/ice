// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * The exception that is thrown when a property cannot be set or retrieved.
 * For example, this exception is thrown when attempting to set an unknown Ice property.
 */
public final class PropertyException extends LocalException {
    private static final long serialVersionUID = 3503752114859671311L;

    /**
     * Constructs a PropertyException with a message.
     *
     * @param message the detail message
     */
    public PropertyException(String message) {
        super(message);
    }

    @Override
    public String ice_id() {
        return "::Ice::PropertyException";
    }
}
