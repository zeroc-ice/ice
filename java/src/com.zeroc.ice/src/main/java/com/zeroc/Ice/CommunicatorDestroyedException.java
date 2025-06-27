// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * The exception that is thrown when an operation fails because the communicator has been destroyed.
 *
 * @see Communicator#destroy
 */
public final class CommunicatorDestroyedException extends LocalException {
    /**
     * Constructs a CommunicatorDestroyedException.
     */
    public CommunicatorDestroyedException() {
        super("communicator destroyed");
    }

    @Override
    public String ice_id() {
        return "::Ice::CommunicatorDestroyedException";
    }

    private static final long serialVersionUID = 752535633703849L;
}
