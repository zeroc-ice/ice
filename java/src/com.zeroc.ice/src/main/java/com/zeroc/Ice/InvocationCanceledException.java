// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * This exception indicates that an asynchronous invocation failed because it was canceled
 * explicitly by the user.
 */
public final class InvocationCanceledException extends LocalException {
    public InvocationCanceledException() {
        super("Invocation canceled.");
    }

    public String ice_id() {
        return "::Ice::InvocationCanceledException";
    }

    private static final long serialVersionUID = -6429704142271073768L;
}
