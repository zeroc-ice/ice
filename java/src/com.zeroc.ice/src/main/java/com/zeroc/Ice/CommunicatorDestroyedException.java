// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * This exception is raised if the {@link Communicator} has been destroyed.
 *
 * @see Communicator#destroy
 */
public final class CommunicatorDestroyedException extends LocalException {
    /**
     * Creates a new {@code CommunicatorDestroyedException}.
     */
    public CommunicatorDestroyedException() {
        super("communicator destroyed");
    }

    public String ice_id() {
        return "::Ice::CommunicatorDestroyedException";
    }

    private static final long serialVersionUID = 752535633703849L;
}
