// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * The exception that is thrown when you attempt to register an object more than once with the Ice runtime.
 */
public final class AlreadyRegisteredException extends LocalException {
    /**
     * Constructs an AlreadyRegisteredException with the specified kind of object and ID.
     *
     * @param kindOfObject the kind of object that could not be removed
     * @param id the ID (or name) of the object that is already registered
     */
    public AlreadyRegisteredException(String kindOfObject, String id) {
        super("Another " + kindOfObject + " is already registered with ID '" + id + "'.");
        this.kindOfObject = kindOfObject;
        this.id = id;
    }

    @Override
    public String ice_id() {
        return "::Ice::AlreadyRegisteredException";
    }

    /**
     * The kind of object that could not be removed: "servant", "facet", "object", "default servant", "servant locator",
     * "plugin", "object adapter", "object adapter with router", "replica group".
     */
    public final String kindOfObject;

    /** The ID (or name) of the object that is already registered. */
    public final String id;

    private static final long serialVersionUID = 392587231034664196L;
}
