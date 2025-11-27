// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * The exception that is thrown when attempting to use an {@link ObjectAdapter} that has been deactivated.
 *
 * @see ObjectAdapter#deactivate
 * @see Communicator#shutdown
 */
public final class ObjectAdapterDeactivatedException extends LocalException {
    /**
     * Constructs an ObjectAdapterDeactivatedException.
     *
     * @param name the name of the deactivated object adapter
     */
    public ObjectAdapterDeactivatedException(String name) {
        super("Object adapter '" + name + "' is deactivated.");
        this.name = name;
    }

    @Override
    public String ice_id() {
        return "::Ice::ObjectAdapterDeactivatedException";
    }

    /** Name of the object adapter. */
    public final String name;

    private static final long serialVersionUID = -1946575462194055987L;
}
