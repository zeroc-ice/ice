// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * An attempt was made to register something more than once with the Ice run time. This exception is
 * raised if an attempt is made to register a servant, servant locator, facet,
 * plug-in, or object adapter more than once for the same ID.
 */
public final class AlreadyRegisteredException extends LocalException {
    public AlreadyRegisteredException(String kindOfObject, String id) {
        super("Another " + kindOfObject + " is already registered with ID '" + id + "'.");
        this.kindOfObject = kindOfObject;
        this.id = id;
    }

    public String ice_id() {
        return "::Ice::AlreadyRegisteredException";
    }

    /**
     * The kind of object that could not be removed: "servant", "facet", "object", "default
     * servant", "servant locator", "plugin", "object adapter", "object adapter
     * with router", "replica group".
     */
    public final String kindOfObject;

    /** The ID (or name) of the object that is registered already. */
    public final String id;

    private static final long serialVersionUID = 392587231034664196L;
}
