// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * This exception is raised when there is an error while getting or setting a property. For example,
 * when trying to set an unknown Ice property.
 */
public final class PropertyException extends LocalException {
    private static final long serialVersionUID = 3503752114859671311L;

    /**
     * Constructs a PropertyException with a message.
     *
     * @param message the detail message for this exception
     */
    public PropertyException(String message) {
        super(message);
    }

    @Override
    public String ice_id() {
        return "::Ice::PropertyException";
    }
}
