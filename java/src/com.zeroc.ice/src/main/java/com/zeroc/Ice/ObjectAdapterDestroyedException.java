// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * The exception that is thrown when attempting to use an {@link ObjectAdapter} that has been destroyed.
 */
public final class ObjectAdapterDestroyedException extends LocalException {
    /**
     * Constructs an ObjectAdapterDestroyedException.
     *
     * @param name the name of the destroyed adapter
     */
    public ObjectAdapterDestroyedException(String name) {
        super("Object adapter '" + name + "' is destroyed.");
        this.name = name;
    }

    @Override
    public String ice_id() {
        return "::Ice::ObjectAdapterDestroyedException";
    }

    /** Name of the adapter. */
    public final String name;

    private static final long serialVersionUID = -1946575462194055987L;
}
