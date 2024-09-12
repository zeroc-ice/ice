// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** The base class for Ice protocol exceptions. */
public class ProtocolException extends LocalException {
    public ProtocolException(String message) {
        super(message);
    }

    public ProtocolException(String message, Throwable cause) {
        super(message, cause);
    }

    public String ice_id() {
        return "::Ice::ProtocolException";
    }

    private static final long serialVersionUID = 6046324714700663302L;
}
