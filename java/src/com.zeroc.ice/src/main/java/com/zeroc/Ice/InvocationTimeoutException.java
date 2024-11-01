// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** This exception indicates that an invocation failed because it timed out. */
public final class InvocationTimeoutException extends TimeoutException {
    public InvocationTimeoutException() {
        super("Invocation timed out.");
    }

    public String ice_id() {
        return "::Ice::InvocationTimeoutException";
    }

    private static final long serialVersionUID = -4956443780705036860L;
}
