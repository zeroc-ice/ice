// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** An unknown property was used when trying to get or set an unknown property. */
public final class UnknownPropertyException extends LocalException {
    private static final long serialVersionUID = 3503752114859671311L;

    public UnknownPropertyException(String message) {
        super(message);
    }

    public String ice_id() {
        return "::Ice::UnknownPropertyException";
    }
}
