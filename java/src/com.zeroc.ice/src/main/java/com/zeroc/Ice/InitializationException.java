// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** This exception is raised when a failure occurs during initialization. */
public final class InitializationException extends LocalException {
    public InitializationException(String message) {
        super(message);
    }

    public InitializationException(String message, Throwable cause) {
        super(message, cause);
    }

    public String ice_id() {
        return "::Ice::InitializationException";
    }

    private static final long serialVersionUID = 578611869232039264L;
}
