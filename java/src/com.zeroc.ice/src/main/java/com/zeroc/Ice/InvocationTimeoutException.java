// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** The exception that is thrown when an invocation times out. */
public final class InvocationTimeoutException extends TimeoutException {
    /**
     * Constructs an InvocationTimeoutException.
     */
    public InvocationTimeoutException() {
        super("Invocation timed out.");
    }

    @Override
    public String ice_id() {
        return "::Ice::InvocationTimeoutException";
    }

    private static final long serialVersionUID = -4956443780705036860L;
}
