// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * This exception is used internally to propagate an invocation cancellation. This exception is created by a call to
 * cancel() on the future returned by an asynchronous invocation.
 */
final class InvocationCanceledException extends LocalException {
    InvocationCanceledException() {
        super("Invocation canceled.");
    }

    public String ice_id() {
        return "::Ice::InvocationCanceledException";
    }

    private static final long serialVersionUID = -6429704142271073768L;
}
