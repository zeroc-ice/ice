// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** This exception is raised for errors during marshaling or unmarshaling. */
public final class MarshalException extends ProtocolException {
    public MarshalException(String message) {
        super(message);
    }

    public MarshalException(String message, Throwable cause) {
        super(message, cause);
    }

    public String ice_id() {
        return "::Ice::MarshalException";
    }

    private static final long serialVersionUID = -1332260000897066889L;
}
