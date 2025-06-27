// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * The exception that is thrown when an asynchronous invocation fails because it was canceled explicitly by the
 * user.
 */
final class InvocationCanceledException extends LocalException {
    /**
     * Constructs an InvocationCanceledException.
     */
    InvocationCanceledException() {
        super("Invocation canceled.");
    }

    @Override
    public String ice_id() {
        return "::Ice::InvocationCanceledException";
    }

    private static final long serialVersionUID = -6429704142271073768L;
}
