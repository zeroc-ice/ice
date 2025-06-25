// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** This exception is raised if an attempt is made to use a destroyed {@link ObjectAdapter}. */
public final class ObjectAdapterDestroyedException extends LocalException {
    /**
     * Constructs an ObjectAdapterDestroyedException with the adapter name.
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
